#include "hashmap.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Hashmap entry structure
typedef struct entry {
    void *key;
    void *value;
    struct entry *next;  // For chaining
} entry_t;

// Hashmap structure
struct hashmap {
    entry_t **buckets;
    size_t capacity;
    size_t size;
    size_t key_size;  // 0 means using custom functions, >0 means using generic byte-wise functions
    hash_func_t hash_func;
    key_compare_func_t key_compare;
    key_free_func_t key_free;
    value_free_func_t value_free;
};

// Helper hash function for null-terminated C strings (djb2 algorithm)
size_t hashmap_string_hash(const void *key) {
    const char *str = (const char *)key;
    size_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

// Helper comparison function for null-terminated C strings
int hashmap_string_compare(const void *key1, const void *key2) {
    return strcmp((const char *)key1, (const char *)key2);
}

// Generic byte-wise hash function (FNV-1a algorithm)
static size_t generic_hash(const void *key, size_t key_size) {
    const uint8_t *data = (const uint8_t *)key;
    size_t hash = 2166136261UL;  // FNV offset basis
    
    for (size_t i = 0; i < key_size; i++) {
        hash ^= data[i];
        hash *= 16777619UL;  // FNV prime
    }
    
    return hash;
}

// Generic byte-wise comparison function
static int generic_compare(const void *key1, const void *key2, size_t key_size) {
    return memcmp(key1, key2, key_size);
}

// Create a new hashmap
hashmap_t *hashmap_create(
    size_t initial_capacity,
    hash_func_t hash_func,
    key_compare_func_t key_compare,
    key_free_func_t key_free,
    value_free_func_t value_free
) {
    if (initial_capacity == 0) {
        initial_capacity = 16; // Default capacity
    }

    hashmap_t *map = (hashmap_t *)malloc(sizeof(hashmap_t));
    if (!map) {
        return NULL;
    }

    map->buckets = (entry_t **)calloc(initial_capacity, sizeof(entry_t *));
    if (!map->buckets) {
        free(map);
        return NULL;
    }

    // Validate required functions
    if (!hash_func || !key_compare) {
        free(map->buckets);
        free(map);
        return NULL;
    }

    map->capacity = initial_capacity;
    map->size = 0;
    map->key_size = 0;  // 0 means using custom functions
    map->hash_func = hash_func;
    map->key_compare = key_compare;
    map->key_free = key_free;
    map->value_free = value_free;

    return map;
}

// Create a new hashmap with generic byte-wise hashing and comparison
hashmap_t *hashmap_create_with_key_size(
    size_t initial_capacity,
    size_t key_size,
    key_free_func_t key_free,
    value_free_func_t value_free
) {
    if (initial_capacity == 0) {
        initial_capacity = 16; // Default capacity
    }
    
    if (key_size == 0) {
        return NULL;  // key_size must be > 0
    }

    hashmap_t *map = (hashmap_t *)malloc(sizeof(hashmap_t));
    if (!map) {
        return NULL;
    }

    map->buckets = (entry_t **)calloc(initial_capacity, sizeof(entry_t *));
    if (!map->buckets) {
        free(map);
        return NULL;
    }

    map->capacity = initial_capacity;
    map->size = 0;
    map->key_size = key_size;  // >0 means using generic byte-wise functions
    map->hash_func = NULL;  // Not used when key_size > 0
    map->key_compare = NULL;  // Not used when key_size > 0
    map->key_free = key_free;
    map->value_free = value_free;

    return map;
}

// Destroy the hashmap and free all resources
void hashmap_destroy(hashmap_t *map) {
    if (!map) {
        return;
    }

    hashmap_clear(map);
    free(map->buckets);
    free(map);
}

// Clear all entries from the hashmap
void hashmap_clear(hashmap_t *map) {
    if (!map) {
        return;
    }

    for (size_t i = 0; i < map->capacity; i++) {
        entry_t *entry = map->buckets[i];
        while (entry) {
            entry_t *next = entry->next;
            
            if (map->key_free) {
                map->key_free(entry->key);
            }
            if (map->value_free) {
                map->value_free(entry->value);
            }
            
            free(entry);
            entry = next;
        }
        map->buckets[i] = NULL;
    }
    
    map->size = 0;
}

// Resize the hashmap (internal function)
static bool hashmap_resize(hashmap_t *map, size_t new_capacity) {
    entry_t **new_buckets = (entry_t **)calloc(new_capacity, sizeof(entry_t *));
    if (!new_buckets) {
        return false;
    }

    // Rehash all entries
    for (size_t i = 0; i < map->capacity; i++) {
        entry_t *entry = map->buckets[i];
        while (entry) {
            entry_t *next = entry->next;
            
            // Calculate new bucket index
            size_t hash = map->key_size > 0 
                ? generic_hash(entry->key, map->key_size)
                : map->hash_func(entry->key);
            size_t new_index = hash % new_capacity;
            
            // Insert into new bucket
            entry->next = new_buckets[new_index];
            new_buckets[new_index] = entry;
            
            entry = next;
        }
    }

    free(map->buckets);
    map->buckets = new_buckets;
    map->capacity = new_capacity;
    
    return true;
}

// Insert or update a key-value pair
bool hashmap_put(hashmap_t *map, void *key, void *value) {
    if (!map || !key) {
        return false;
    }

    // Resize if load factor exceeds 0.75
    if (map->size >= map->capacity * 3 / 4) {
        if (!hashmap_resize(map, map->capacity * 2)) {
            return false;
        }
    }

    size_t hash = map->key_size > 0 
        ? generic_hash(key, map->key_size)
        : map->hash_func(key);
    size_t index = hash % map->capacity;

    // Check if key already exists
    entry_t *entry = map->buckets[index];
    while (entry) {
        int cmp_result = map->key_size > 0
            ? generic_compare(entry->key, key, map->key_size)
            : map->key_compare(entry->key, key);
        if (cmp_result == 0) {
            // Update existing entry
            if (map->value_free && entry->value != value) {
                map->value_free(entry->value);
            }
            entry->value = value;
            return true;
        }
        entry = entry->next;
    }

    // Create new entry
    entry = (entry_t *)malloc(sizeof(entry_t));
    if (!entry) {
        return false;
    }

    entry->key = key;
    entry->value = value;
    entry->next = map->buckets[index];
    map->buckets[index] = entry;
    map->size++;

    return true;
}

// Get the value associated with a key
void *hashmap_get(const hashmap_t *map, const void *key) {
    if (!map || !key) {
        return NULL;
    }

    size_t hash = map->key_size > 0 
        ? generic_hash(key, map->key_size)
        : map->hash_func(key);
    size_t index = hash % map->capacity;

    entry_t *entry = map->buckets[index];
    while (entry) {
        int cmp_result = map->key_size > 0
            ? generic_compare(entry->key, key, map->key_size)
            : map->key_compare(entry->key, key);
        if (cmp_result == 0) {
            return entry->value;
        }
        entry = entry->next;
    }

    return NULL;
}

// Remove a key-value pair
bool hashmap_remove(hashmap_t *map, const void *key) {
    if (!map || !key) {
        return false;
    }

    size_t hash = map->key_size > 0 
        ? generic_hash(key, map->key_size)
        : map->hash_func(key);
    size_t index = hash % map->capacity;

    entry_t *entry = map->buckets[index];
    entry_t *prev = NULL;

    while (entry) {
        int cmp_result = map->key_size > 0
            ? generic_compare(entry->key, key, map->key_size)
            : map->key_compare(entry->key, key);
        if (cmp_result == 0) {
            // Remove entry from chain
            if (prev) {
                prev->next = entry->next;
            } else {
                map->buckets[index] = entry->next;
            }

            // Free key and value if needed
            if (map->key_free) {
                map->key_free(entry->key);
            }
            if (map->value_free) {
                map->value_free(entry->value);
            }

            free(entry);
            map->size--;
            return true;
        }
        prev = entry;
        entry = entry->next;
    }

    return false;
}

// Check if a key exists in the hashmap
bool hashmap_contains(const hashmap_t *map, const void *key) {
    return hashmap_get(map, key) != NULL;
}

// Get the number of key-value pairs in the hashmap
size_t hashmap_size(const hashmap_t *map) {
    return map ? map->size : 0;
}

// Check if the hashmap is empty
bool hashmap_is_empty(const hashmap_t *map) {
    return map ? map->size == 0 : true;
}

