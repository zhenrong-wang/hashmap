#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashmap.h"

// Example 1: Using string keys (using helper functions)
void example_string_keys() {
    printf("=== Example 1: String Keys ===\n");
    
    // Create hashmap with string hash/compare helper functions
    hashmap_t *map = hashmap_create(16, hashmap_string_hash, hashmap_string_compare, NULL, NULL);
    
    // Insert some key-value pairs
    hashmap_put(map, "apple", "red");
    hashmap_put(map, "banana", "yellow");
    hashmap_put(map, "grape", "purple");
    hashmap_put(map, "orange", "orange");
    
    printf("Size: %zu\n", hashmap_size(map));
    
    // Get values
    printf("apple -> %s\n", (char *)hashmap_get(map, "apple"));
    printf("banana -> %s\n", (char *)hashmap_get(map, "banana"));
    printf("grape -> %s\n", (char *)hashmap_get(map, "grape"));
    
    // Check if key exists
    printf("Contains 'apple': %s\n", hashmap_contains(map, "apple") ? "yes" : "no");
    printf("Contains 'mango': %s\n", hashmap_contains(map, "mango") ? "yes" : "no");
    
    // Update existing key
    hashmap_put(map, "apple", "green");
    printf("apple -> %s (after update)\n", (char *)hashmap_get(map, "apple"));
    
    // Remove a key
    hashmap_remove(map, "banana");
    printf("Size after removing 'banana': %zu\n", hashmap_size(map));
    printf("Contains 'banana': %s\n", hashmap_contains(map, "banana") ? "yes" : "no");
    
    hashmap_destroy(map);
    printf("\n");
}

// Example 2: Using integer keys
void example_int_keys() {
    printf("=== Example 2: Integer Keys ===\n");
    
    // Create hashmap with integer hash/compare helper functions
    hashmap_t *map = hashmap_create(16, hashmap_int_hash, hashmap_int_compare, NULL, NULL);
    
    // Allocate keys and values
    int *key1 = malloc(sizeof(int));
    int *key2 = malloc(sizeof(int));
    int *key3 = malloc(sizeof(int));
    *key1 = 10;
    *key2 = 20;
    *key3 = 30;
    
    int *val1 = malloc(sizeof(int));
    int *val2 = malloc(sizeof(int));
    int *val3 = malloc(sizeof(int));
    *val1 = 100;
    *val2 = 200;
    *val3 = 300;
    
    hashmap_put(map, key1, val1);
    hashmap_put(map, key2, val2);
    hashmap_put(map, key3, val3);
    
    printf("Size: %zu\n", hashmap_size(map));
    
    // Get values
    int lookup_key = 20;
    int *result = (int *)hashmap_get(map, &lookup_key);
    if (result) {
        printf("Key 20 -> Value %d\n", *result);
    }
    
    lookup_key = 10;
    result = (int *)hashmap_get(map, &lookup_key);
    if (result) {
        printf("Key 10 -> Value %d\n", *result);
    }
    
    // Cleanup
    hashmap_destroy(map);
    printf("\n");
}

// Example 3: Using generic key_size (no need for custom hash/compare functions!)
void example_generic_key_size() {
    printf("=== Example 3: Generic Key Size (No Custom Functions Needed!) ===\n");
    
    // Create hashmap with key_size - works for any fixed-size type!
    // No need to write hash/compare functions for integers, structs, etc.
    hashmap_t *map = hashmap_create_with_key_size(16, sizeof(int), NULL, NULL);
    
    // Allocate keys and values
    int *key1 = malloc(sizeof(int));
    int *key2 = malloc(sizeof(int));
    int *key3 = malloc(sizeof(int));
    *key1 = 10;
    *key2 = 20;
    *key3 = 30;
    
    int *val1 = malloc(sizeof(int));
    int *val2 = malloc(sizeof(int));
    int *val3 = malloc(sizeof(int));
    *val1 = 100;
    *val2 = 200;
    *val3 = 300;
    
    hashmap_put(map, key1, val1);
    hashmap_put(map, key2, val2);
    hashmap_put(map, key3, val3);
    
    printf("Size: %zu\n", hashmap_size(map));
    
    // Get values - just pass pointer to the key value
    int lookup_key = 20;
    int *result = (int *)hashmap_get(map, &lookup_key);
    if (result) {
        printf("Key 20 -> Value %d\n", *result);
    }
    
    lookup_key = 10;
    result = (int *)hashmap_get(map, &lookup_key);
    if (result) {
        printf("Key 10 -> Value %d\n", *result);
    }
    
    // Works with structs too! No custom functions needed
    typedef struct {
        int x;
        int y;
    } point_t;
    
    hashmap_t *point_map = hashmap_create_with_key_size(16, sizeof(point_t), NULL, NULL);
    
    point_t *p1 = malloc(sizeof(point_t));
    point_t *p2 = malloc(sizeof(point_t));
    p1->x = 1; p1->y = 2;
    p2->x = 3; p2->y = 4;
    
    char *label1 = "origin";
    char *label2 = "target";
    
    hashmap_put(point_map, p1, label1);
    hashmap_put(point_map, p2, label2);
    
    point_t lookup = {1, 2};
    char *found = (char *)hashmap_get(point_map, &lookup);
    if (found) {
        printf("Point (1,2) -> %s\n", found);
    }
    
    hashmap_destroy(map);
    hashmap_destroy(point_map);
    printf("\n");
}

// Example 4: With memory management
void example_with_cleanup() {
    printf("=== Example 4: With Memory Cleanup ===\n");
    
    // Create hashmap with string helpers and cleanup functions
    hashmap_t *map = hashmap_create(16, hashmap_string_hash, hashmap_string_compare, free, free);
    
    // Insert dynamically allocated strings
    char *key1 = strdup("name");
    char *key2 = strdup("age");
    char *key3 = strdup("city");
    
    char *val1 = strdup("John");
    char *val2 = strdup("30");
    char *val3 = strdup("New York");
    
    hashmap_put(map, key1, val1);
    hashmap_put(map, key2, val2);
    hashmap_put(map, key3, val3);
    
    printf("Size: %zu\n", hashmap_size(map));
    printf("name -> %s\n", (char *)hashmap_get(map, "name"));
    printf("age -> %s\n", (char *)hashmap_get(map, "age"));
    printf("city -> %s\n", (char *)hashmap_get(map, "city"));
    
    // hashmap_destroy will automatically free all keys and values
    hashmap_destroy(map);
    printf("All memory cleaned up automatically\n");
    printf("\n");
}

int main() {
    example_string_keys();
    example_int_keys();
    example_generic_key_size();
    example_with_cleanup();
    
    return 0;
}

