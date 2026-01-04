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
    hash_func_t hash_func;
    key_compare_func_t key_compare;
    key_free_func_t key_free;
    value_free_func_t value_free;
};

// Default hash function (djb2 algorithm for strings)
size_t hashmap_default_hash(const void *key) {
    const char *str = (const char *)key;
    size_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

// Default key comparison function for strings
int hashmap_default_key_compare(const void *key1, const void *key2) {
    return strcmp((const char *)key1, (const char *)key2);
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

    map->capacity = initial_capacity;
    map->size = 0;
    map->hash_func = hash_func ? hash_func : hashmap_default_hash;
    map->key_compare = key_compare ? key_compare : hashmap_default_key_compare;
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
            size_t hash = map->hash_func(entry->key);
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

    size_t hash = map->hash_func(key);
    size_t index = hash % map->capacity;

    // Check if key already exists
    entry_t *entry = map->buckets[index];
    while (entry) {
        if (map->key_compare(entry->key, key) == 0) {
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

    size_t hash = map->hash_func(key);
    size_t index = hash % map->capacity;

    entry_t *entry = map->buckets[index];
    while (entry) {
        if (map->key_compare(entry->key, key) == 0) {
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

    size_t hash = map->hash_func(key);
    size_t index = hash % map->capacity;

    entry_t *entry = map->buckets[index];
    entry_t *prev = NULL;

    while (entry) {
        if (map->key_compare(entry->key, key) == 0) {
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

