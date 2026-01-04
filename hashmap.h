#ifndef HASHMAP_H
#define HASHMAP_H

#include <stddef.h>
#include <stdbool.h>

// Hashmap structure
typedef struct hashmap hashmap_t;

// Function pointer types
typedef size_t (*hash_func_t)(const void *key);
typedef int (*key_compare_func_t)(const void *key1, const void *key2);
typedef void (*key_free_func_t)(void *key);
typedef void (*value_free_func_t)(void *value);

// Create a new hashmap
// initial_capacity: initial number of buckets (must be > 0)
// hash_func: function to hash keys (REQUIRED - cannot be NULL)
// key_compare: function to compare keys (REQUIRED - cannot be NULL)
// key_free: function to free keys (NULL for no cleanup)
// value_free: function to free values (NULL for no cleanup)
// Returns NULL on failure
hashmap_t *hashmap_create(
    size_t initial_capacity,
    hash_func_t hash_func,
    key_compare_func_t key_compare,
    key_free_func_t key_free,
    value_free_func_t value_free
);

// Create a new hashmap with generic byte-wise hashing and comparison
// This is convenient for fixed-size key types (int, structs, binary data, etc.)
// initial_capacity: initial number of buckets (must be > 0)
// key_size: size of each key in bytes (must be > 0)
// key_free: function to free keys (NULL for no cleanup)
// value_free: function to free values (NULL for no cleanup)
// Returns NULL on failure
hashmap_t *hashmap_create_with_key_size(
    size_t initial_capacity,
    size_t key_size,
    key_free_func_t key_free,
    value_free_func_t value_free
);

// Destroy the hashmap and free all resources
void hashmap_destroy(hashmap_t *map);

// Insert or update a key-value pair
// Returns true on success, false on failure
bool hashmap_put(hashmap_t *map, void *key, void *value);

// Get the value associated with a key
// Returns the value if found, NULL otherwise
void *hashmap_get(const hashmap_t *map, const void *key);

// Remove a key-value pair
// Returns true if the key was found and removed, false otherwise
bool hashmap_remove(hashmap_t *map, const void *key);

// Check if a key exists in the hashmap
bool hashmap_contains(const hashmap_t *map, const void *key);

// Get the number of key-value pairs in the hashmap
size_t hashmap_size(const hashmap_t *map);

// Check if the hashmap is empty
bool hashmap_is_empty(const hashmap_t *map);

// Clear all entries from the hashmap
void hashmap_clear(hashmap_t *map);

// Helper functions for variable-length keys

// Hash function for null-terminated C strings
// Note: For fixed-size types (int, structs, etc.), use hashmap_create_with_key_size() instead
size_t hashmap_string_hash(const void *key);

// Comparison function for null-terminated C strings
// Note: For fixed-size types (int, structs, etc.), use hashmap_create_with_key_size() instead
int hashmap_string_compare(const void *key1, const void *key2);

#endif // HASHMAP_H

