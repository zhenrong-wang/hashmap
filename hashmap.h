#ifndef HASHMAP_H
#define HASHMAP_H

#include <stddef.h>
#include <stdbool.h>

// Hashmap structure
typedef struct hashmap hashmap_t;

// Function pointer types for cleanup
typedef void (*key_free_func_t)(void *key);
typedef void (*value_free_func_t)(void *value);

// Create a new hashmap
// initial_capacity: initial number of buckets (must be > 0, defaults to 16)
// key_free: function to free keys (NULL for no cleanup - keys are always copied internally)
// value_free: function to free values (NULL for no cleanup)
// Returns NULL on failure
hashmap_t *hashmap_create(
    size_t initial_capacity,
    key_free_func_t key_free,
    value_free_func_t value_free
);

// Destroy the hashmap and free all resources
void hashmap_destroy(hashmap_t *map);

// Insert or update a key-value pair
// key: pointer to key data (will be copied)
// key_size: size of key in bytes (must be > 0)
// value: pointer to value (stored as-is, not copied)
// Returns true on success, false on failure
bool hashmap_put(hashmap_t *map, const void *key, size_t key_size, void *value);

// Get the value associated with a key
// key: pointer to key data
// key_size: size of key in bytes (must match the size used when inserting)
// Returns the value if found, NULL otherwise
void *hashmap_get(const hashmap_t *map, const void *key, size_t key_size);

// Remove a key-value pair
// key: pointer to key data
// key_size: size of key in bytes (must match the size used when inserting)
// Returns true if the key was found and removed, false otherwise
bool hashmap_remove(hashmap_t *map, const void *key, size_t key_size);

// Check if a key exists in the hashmap
bool hashmap_contains(const hashmap_t *map, const void *key, size_t key_size);

// Get the number of key-value pairs in the hashmap
size_t hashmap_size(const hashmap_t *map);

// Check if the hashmap is empty
bool hashmap_is_empty(const hashmap_t *map);

// Clear all entries from the hashmap
void hashmap_clear(hashmap_t *map);

#endif // HASHMAP_H

