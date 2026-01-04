#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "hashmap.h"

// Example 1: Integer keys
void example_int_keys() {
    printf("=== Example 1: Integer Keys ===\n");
    
    hashmap_t *map = hashmap_create(16, NULL, NULL);
    
    int key1 = 10, key2 = 20, key3 = 30;
    int val1 = 100, val2 = 200, val3 = 300;
    
    hashmap_put(map, &key1, sizeof(int), &val1);
    hashmap_put(map, &key2, sizeof(int), &val2);
    hashmap_put(map, &key3, sizeof(int), &val3);
    
    printf("Size: %zu\n", hashmap_size(map));
    
    int lookup_key = 20;
    int *result = (int *)hashmap_get(map, &lookup_key, sizeof(int));
    if (result) {
        printf("Key 20 -> Value %d\n", *result);
    }
    
    hashmap_destroy(map);
    printf("\n");
}

// Example 2: String keys
void example_string_keys() {
    printf("=== Example 2: String Keys ===\n");
    
    hashmap_t *map = hashmap_create(16, NULL, NULL);
    
    hashmap_put(map, "apple", strlen("apple") + 1, "red");
    hashmap_put(map, "banana", strlen("banana") + 1, "yellow");
    hashmap_put(map, "grape", strlen("grape") + 1, "purple");
    
    printf("Size: %zu\n", hashmap_size(map));
    
    printf("apple -> %s\n", (char *)hashmap_get(map, "apple", strlen("apple") + 1));
    printf("banana -> %s\n", (char *)hashmap_get(map, "banana", strlen("banana") + 1));
    
    hashmap_destroy(map);
    printf("\n");
}

// Example 3: Struct keys
void example_struct_keys() {
    printf("=== Example 3: Struct Keys ===\n");
    
    typedef struct {
        int x;
        int y;
    } point_t;
    
    hashmap_t *map = hashmap_create(16, NULL, NULL);
    
    point_t p1 = {1, 2};
    point_t p2 = {3, 4};
    
    hashmap_put(map, &p1, sizeof(point_t), "origin");
    hashmap_put(map, &p2, sizeof(point_t), "target");
    
    point_t lookup = {1, 2};
    char *result = (char *)hashmap_get(map, &lookup, sizeof(point_t));
    if (result) {
        printf("Point (1,2) -> %s\n", result);
    }
    
    hashmap_destroy(map);
    printf("\n");
}

// Example 4: Variable-length binary data
void example_binary_keys() {
    printf("=== Example 4: Variable-Length Binary Keys ===\n");
    
    hashmap_t *map = hashmap_create(16, NULL, NULL);
    
    uint8_t key1[] = {0x01, 0x02, 0x03};
    uint8_t key2[] = {0x04, 0x05, 0x06, 0x07, 0x08};  // Different length!
    uint8_t key3[] = {0x00, 0x01, 0x00, 0x02};  // Even with null bytes!
    
    hashmap_put(map, key1, sizeof(key1), "value1");
    hashmap_put(map, key2, sizeof(key2), "value2");
    hashmap_put(map, key3, sizeof(key3), "value3");
    
    printf("Size: %zu\n", hashmap_size(map));
    
    uint8_t lookup[] = {0x01, 0x02, 0x03};
    char *result = (char *)hashmap_get(map, lookup, sizeof(lookup));
    if (result) {
        printf("Key [0x01, 0x02, 0x03] -> %s\n", result);
    }
    
    hashmap_destroy(map);
    printf("\n");
}

int main() {
    example_int_keys();
    example_string_keys();
    example_struct_keys();
    example_binary_keys();
    
    return 0;
}

