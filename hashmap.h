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

// Helper hash functions for common key types

// Hash function for null-terminated C strings
size_t hashmap_string_hash(const void *key);

// Hash function for integer keys (pass pointer to int)
size_t hashmap_int_hash(const void *key);

// Hash function for pointer keys (uses pointer address)
size_t hashmap_ptr_hash(const void *key);

// Hash function for binary data (requires key_size parameter)
// Note: This is a helper - you need to create a wrapper that captures key_size
// Example: Use hashmap_binary_hash_with_size(key_size) to get a hash function

// Helper comparison functions for common key types

// Comparison function for null-terminated C strings
int hashmap_string_compare(const void *key1, const void *key2);

// Comparison function for integer keys (pass pointers to int)
int hashmap_int_compare(const void *key1, const void *key2);

// Comparison function for pointer keys (compares pointer addresses)
int hashmap_ptr_compare(const void *key1, const void *key2);

#endif // HASHMAP_H

