#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>
#include "hashmap.h"

// Test statistics
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

// Test macros
#define TEST_START(name) \
    do { \
        printf("TEST: %s\n", name); \
        tests_run++; \
    } while(0)

#define TEST_PASS() \
    do { \
        printf("  ✓ PASSED\n"); \
        tests_passed++; \
    } while(0)

#define TEST_FAIL(msg) \
    do { \
        printf("  ✗ FAILED: %s\n", msg); \
        tests_failed++; \
        return false; \
    } while(0)

#define ASSERT(condition, msg) \
    do { \
        if (!(condition)) { \
            TEST_FAIL(msg); \
        } \
    } while(0)

// Test function prototype
typedef bool (*test_func_t)(void);

// ============================================================================
// API Robustness Tests
// ============================================================================

// Test 1: Basic put and get
bool test_basic_put_get() {
    TEST_START("Basic put and get");
    
    hashmap_t *map = hashmap_create(16, NULL, NULL);
    ASSERT(map != NULL, "hashmap_create failed");
    
    int key = 42;
    int value = 100;
    
    ASSERT(hashmap_put(map, &key, sizeof(int), &value), "hashmap_put failed");
    ASSERT(hashmap_size(map) == 1, "Size should be 1");
    
    int *result = (int *)hashmap_get(map, &key, sizeof(int));
    ASSERT(result != NULL, "hashmap_get returned NULL");
    ASSERT(*result == 100, "Value mismatch");
    
    hashmap_destroy(map);
    TEST_PASS();
    return true;
}

// Test 2: Update existing key
bool test_update_key() {
    TEST_START("Update existing key");
    
    hashmap_t *map = hashmap_create(16, NULL, NULL);
    int key = 10;
    int value1 = 100;
    int value2 = 200;
    
    hashmap_put(map, &key, sizeof(int), &value1);
    hashmap_put(map, &key, sizeof(int), &value2);
    
    ASSERT(hashmap_size(map) == 1, "Size should still be 1 after update");
    
    int *result = (int *)hashmap_get(map, &key, sizeof(int));
    ASSERT(result != NULL, "hashmap_get returned NULL");
    ASSERT(*result == 200, "Value should be updated");
    
    hashmap_destroy(map);
    TEST_PASS();
    return true;
}

// Test 3: Remove key
bool test_remove_key() {
    TEST_START("Remove key");
    
    hashmap_t *map = hashmap_create(16, NULL, NULL);
    int key = 42;
    int value = 100;
    
    hashmap_put(map, &key, sizeof(int), &value);
    ASSERT(hashmap_size(map) == 1, "Size should be 1");
    
    ASSERT(hashmap_remove(map, &key, sizeof(int)), "hashmap_remove failed");
    ASSERT(hashmap_size(map) == 0, "Size should be 0 after remove");
    
    int *result = (int *)hashmap_get(map, &key, sizeof(int));
    ASSERT(result == NULL, "Key should not exist after remove");
    
    hashmap_destroy(map);
    TEST_PASS();
    return true;
}

// Test 4: Contains check
bool test_contains() {
    TEST_START("Contains check");
    
    hashmap_t *map = hashmap_create(16, NULL, NULL);
    int key1 = 10;
    int key2 = 20;
    int value = 100;
    
    hashmap_put(map, &key1, sizeof(int), &value);
    
    ASSERT(hashmap_contains(map, &key1, sizeof(int)), "Should contain key1");
    ASSERT(!hashmap_contains(map, &key2, sizeof(int)), "Should not contain key2");
    
    hashmap_destroy(map);
    TEST_PASS();
    return true;
}

// Test 5: Empty hashmap operations
bool test_empty_hashmap() {
    TEST_START("Empty hashmap operations");
    
    hashmap_t *map = hashmap_create(16, NULL, NULL);
    
    ASSERT(hashmap_is_empty(map), "Should be empty");
    ASSERT(hashmap_size(map) == 0, "Size should be 0");
    
    int key = 42;
    int *result = (int *)hashmap_get(map, &key, sizeof(int));
    ASSERT(result == NULL, "get on empty map should return NULL");
    
    ASSERT(!hashmap_remove(map, &key, sizeof(int)), "remove on empty map should return false");
    
    hashmap_destroy(map);
    TEST_PASS();
    return true;
}

// Test 6: String keys
bool test_string_keys() {
    TEST_START("String keys");
    
    hashmap_t *map = hashmap_create(16, NULL, NULL);
    
    hashmap_put(map, "apple", strlen("apple") + 1, "red");
    hashmap_put(map, "banana", strlen("banana") + 1, "yellow");
    
    char *result = (char *)hashmap_get(map, "apple", strlen("apple") + 1);
    ASSERT(result != NULL, "Should find apple");
    ASSERT(strcmp(result, "red") == 0, "Value mismatch");
    
    result = (char *)hashmap_get(map, "banana", strlen("banana") + 1);
    ASSERT(result != NULL, "Should find banana");
    ASSERT(strcmp(result, "yellow") == 0, "Value mismatch");
    
    hashmap_destroy(map);
    TEST_PASS();
    return true;
}

// Test 7: Struct keys
bool test_struct_keys() {
    TEST_START("Struct keys");
    
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
    ASSERT(result != NULL, "Should find point");
    ASSERT(strcmp(result, "origin") == 0, "Value mismatch");
    
    hashmap_destroy(map);
    TEST_PASS();
    return true;
}

// Test 8: Binary data with null bytes
bool test_binary_data() {
    TEST_START("Binary data with null bytes");
    
    hashmap_t *map = hashmap_create(16, NULL, NULL);
    
    uint8_t key1[] = {0x00, 0x01, 0x00, 0x02};
    uint8_t key2[] = {0xFF, 0xFE, 0xFD};
    
    hashmap_put(map, key1, sizeof(key1), "value1");
    hashmap_put(map, key2, sizeof(key2), "value2");
    
    uint8_t lookup[] = {0x00, 0x01, 0x00, 0x02};
    char *result = (char *)hashmap_get(map, lookup, sizeof(lookup));
    ASSERT(result != NULL, "Should find binary key");
    ASSERT(strcmp(result, "value1") == 0, "Value mismatch");
    
    hashmap_destroy(map);
    TEST_PASS();
    return true;
}

// Test 9: Variable length keys
bool test_variable_length_keys() {
    TEST_START("Variable length keys");
    
    hashmap_t *map = hashmap_create(16, NULL, NULL);
    
    uint8_t key1[] = {0x01, 0x02};
    uint8_t key2[] = {0x03, 0x04, 0x05, 0x06, 0x07};
    uint8_t key3[] = {0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
    
    hashmap_put(map, key1, sizeof(key1), "short");
    hashmap_put(map, key2, sizeof(key2), "medium");
    hashmap_put(map, key3, sizeof(key3), "long");
    
    ASSERT(hashmap_size(map) == 3, "Should have 3 entries");
    
    uint8_t lookup[] = {0x03, 0x04, 0x05, 0x06, 0x07};
    char *result = (char *)hashmap_get(map, lookup, sizeof(lookup));
    ASSERT(result != NULL, "Should find medium key");
    ASSERT(strcmp(result, "medium") == 0, "Value mismatch");
    
    hashmap_destroy(map);
    TEST_PASS();
    return true;
}

// Test 10: Large number of entries
bool test_large_insertions() {
    TEST_START("Large number of entries");
    
    hashmap_t *map = hashmap_create(16, NULL, NULL);
    const int N = 10000;
    
    // Allocate values array to avoid stack variable reuse
    int *values = malloc(N * sizeof(int));
    ASSERT(values != NULL, "Memory allocation failed");
    
    // Insert N entries
    for (int i = 0; i < N; i++) {
        int key = i;
        values[i] = i * 10;
        ASSERT(hashmap_put(map, &key, sizeof(int), &values[i]), "Put failed");
    }
    
    ASSERT(hashmap_size(map) == N, "Size should be N");
    
    // Verify all entries
    for (int i = 0; i < N; i++) {
        int key = i;
        int *result = (int *)hashmap_get(map, &key, sizeof(int));
        ASSERT(result != NULL, "Key should exist");
        ASSERT(*result == i * 10, "Value mismatch");
    }
    
    hashmap_destroy(map);
    free(values);
    TEST_PASS();
    return true;
}

// Test 11: Collision handling
bool test_collisions() {
    TEST_START("Collision handling");
    
    hashmap_t *map = hashmap_create(4, NULL, NULL); // Small capacity to force collisions
    
    // Allocate values array to avoid stack variable reuse
    int *values = malloc(100 * sizeof(int));
    ASSERT(values != NULL, "Memory allocation failed");
    
    // Insert multiple entries that will likely collide
    for (int i = 0; i < 100; i++) {
        int key = i;
        values[i] = i * 2;
        ASSERT(hashmap_put(map, &key, sizeof(int), &values[i]), "Put failed");
    }
    
    ASSERT(hashmap_size(map) == 100, "Should have 100 entries");
    
    // Verify all entries still accessible
    for (int i = 0; i < 100; i++) {
        int key = i;
        int *result = (int *)hashmap_get(map, &key, sizeof(int));
        ASSERT(result != NULL, "Key should exist");
        ASSERT(*result == i * 2, "Value mismatch");
    }
    
    hashmap_destroy(map);
    free(values);
    TEST_PASS();
    return true;
}

// Test 12: Resize behavior
bool test_resize() {
    TEST_START("Resize behavior");
    
    hashmap_t *map = hashmap_create(16, NULL, NULL);
    
    // Insert enough entries to trigger resize (load factor > 0.75)
    const int N = 20; // 20 > 16 * 0.75 = 12
    for (int i = 0; i < N; i++) {
        int key = i;
        int value = i;
        ASSERT(hashmap_put(map, &key, sizeof(int), &value), "Put failed");
    }
    
    ASSERT(hashmap_size(map) == N, "Size should be N");
    
    // Verify all entries still accessible after resize
    for (int i = 0; i < N; i++) {
        int key = i;
        int *result = (int *)hashmap_get(map, &key, sizeof(int));
        ASSERT(result != NULL, "Key should exist after resize");
        ASSERT(*result == i, "Value mismatch");
    }
    
    hashmap_destroy(map);
    TEST_PASS();
    return true;
}

// Test 13: Clear operation
bool test_clear() {
    TEST_START("Clear operation");
    
    hashmap_t *map = hashmap_create(16, NULL, NULL);
    
    for (int i = 0; i < 100; i++) {
        int key = i;
        int value = i;
        hashmap_put(map, &key, sizeof(int), &value);
    }
    
    ASSERT(hashmap_size(map) == 100, "Should have 100 entries");
    
    hashmap_clear(map);
    
    ASSERT(hashmap_size(map) == 0, "Size should be 0 after clear");
    ASSERT(hashmap_is_empty(map), "Should be empty");
    
    // Verify entries are gone
    int key = 50;
    int *result = (int *)hashmap_get(map, &key, sizeof(int));
    ASSERT(result == NULL, "Key should not exist after clear");
    
    hashmap_destroy(map);
    TEST_PASS();
    return true;
}

// Test 14: Memory cleanup with value_free
bool test_memory_cleanup() {
    TEST_START("Memory cleanup with value_free");
    
    hashmap_t *map = hashmap_create(16, NULL, free);
    
    char *val1 = strdup("value1");
    char *val2 = strdup("value2");
    char *val3 = strdup("value3");
    
    int key1 = 1, key2 = 2, key3 = 3;
    
    hashmap_put(map, &key1, sizeof(int), val1);
    hashmap_put(map, &key2, sizeof(int), val2);
    hashmap_put(map, &key3, sizeof(int), val3);
    
    // Destroy should free all values
    hashmap_destroy(map);
    
    // Note: We can't verify the memory was freed, but valgrind would catch leaks
    TEST_PASS();
    return true;
}

// Test 15: Invalid parameters
bool test_invalid_parameters() {
    TEST_START("Invalid parameters");
    
    hashmap_t *map = hashmap_create(16, NULL, NULL);
    int key = 42;
    int value = 100;
    
    // NULL map
    ASSERT(!hashmap_put(NULL, &key, sizeof(int), &value), "Should fail with NULL map");
    ASSERT(hashmap_get(NULL, &key, sizeof(int)) == NULL, "Should return NULL with NULL map");
    ASSERT(!hashmap_remove(NULL, &key, sizeof(int)), "Should fail with NULL map");
    
    // NULL key
    ASSERT(!hashmap_put(map, NULL, sizeof(int), &value), "Should fail with NULL key");
    ASSERT(hashmap_get(map, NULL, sizeof(int)) == NULL, "Should return NULL with NULL key");
    
    // Zero key_size
    ASSERT(!hashmap_put(map, &key, 0, &value), "Should fail with zero key_size");
    ASSERT(hashmap_get(map, &key, 0) == NULL, "Should return NULL with zero key_size");
    
    hashmap_destroy(map);
    TEST_PASS();
    return true;
}

// Test 16: Key size mismatch
bool test_key_size_mismatch() {
    TEST_START("Key size mismatch");
    
    hashmap_t *map = hashmap_create(16, NULL, NULL);
    
    int key = 42;
    int value = 100;
    
    hashmap_put(map, &key, sizeof(int), &value);
    
    // Try to get with wrong size
    int *result = (int *)hashmap_get(map, &key, sizeof(int) + 1);
    ASSERT(result == NULL, "Should not find key with wrong size");
    
    result = (int *)hashmap_get(map, &key, sizeof(int) - 1);
    ASSERT(result == NULL, "Should not find key with wrong size");
    
    // Correct size should work
    result = (int *)hashmap_get(map, &key, sizeof(int));
    ASSERT(result != NULL, "Should find key with correct size");
    
    hashmap_destroy(map);
    TEST_PASS();
    return true;
}

// Test 17: Destroy NULL map
bool test_destroy_null() {
    TEST_START("Destroy NULL map");
    
    // Should not crash
    hashmap_destroy(NULL);
    hashmap_clear(NULL);
    
    TEST_PASS();
    return true;
}

// Test 18: Default capacity
bool test_default_capacity() {
    TEST_START("Default capacity");
    
    hashmap_t *map = hashmap_create(0, NULL, NULL);
    ASSERT(map != NULL, "Should create with default capacity");
    
    int key = 42;
    int value = 100;
    ASSERT(hashmap_put(map, &key, sizeof(int), &value), "Should work with default capacity");
    
    hashmap_destroy(map);
    TEST_PASS();
    return true;
}

// ============================================================================
// Performance Tests
// ============================================================================

static double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

// Performance test: Insert performance
bool test_perf_insert() {
    TEST_START("Performance: Insert");
    
    hashmap_t *map = hashmap_create(16, NULL, NULL);
    const int N = 100000;
    
    double start = get_time_ms();
    for (int i = 0; i < N; i++) {
        int key = i;
        int value = i;
        hashmap_put(map, &key, sizeof(int), &value);
    }
    double end = get_time_ms();
    
    double time_ms = end - start;
    double ops_per_sec = (N / time_ms) * 1000.0;
    
    printf("  Inserted %d entries in %.2f ms (%.0f ops/sec)\n", 
           N, time_ms, ops_per_sec);
    
    hashmap_destroy(map);
    TEST_PASS();
    return true;
}

// Performance test: Lookup performance
bool test_perf_lookup() {
    TEST_START("Performance: Lookup");
    
    hashmap_t *map = hashmap_create(16, NULL, NULL);
    const int N = 100000;
    const int LOOKUPS = 1000000;
    
    // Insert N entries
    for (int i = 0; i < N; i++) {
        int key = i;
        int value = i;
        hashmap_put(map, &key, sizeof(int), &value);
    }
    
    // Perform lookups
    double start = get_time_ms();
    int found = 0;
    for (int i = 0; i < LOOKUPS; i++) {
        int key = i % N;
        int *result = (int *)hashmap_get(map, &key, sizeof(int));
        if (result != NULL) found++;
    }
    double end = get_time_ms();
    
    double time_ms = end - start;
    double ops_per_sec = (LOOKUPS / time_ms) * 1000.0;
    
    printf("  Performed %d lookups in %.2f ms (%.0f ops/sec, %d found)\n", 
           LOOKUPS, time_ms, ops_per_sec, found);
    
    hashmap_destroy(map);
    TEST_PASS();
    return true;
}

// Performance test: Mixed operations
bool test_perf_mixed() {
    TEST_START("Performance: Mixed operations");
    
    hashmap_t *map = hashmap_create(16, NULL, NULL);
    const int N = 50000;
    const int OPS = 200000;
    
    double start = get_time_ms();
    for (int i = 0; i < OPS; i++) {
        int key = i % N;
        int value = i;
        
        if (i % 3 == 0) {
            // Insert
            hashmap_put(map, &key, sizeof(int), &value);
        } else if (i % 3 == 1) {
            // Lookup
            hashmap_get(map, &key, sizeof(int));
        } else {
            // Remove
            hashmap_remove(map, &key, sizeof(int));
        }
    }
    double end = get_time_ms();
    
    double time_ms = end - start;
    double ops_per_sec = (OPS / time_ms) * 1000.0;
    
    printf("  Performed %d mixed operations in %.2f ms (%.0f ops/sec)\n", 
           OPS, time_ms, ops_per_sec);
    
    hashmap_destroy(map);
    TEST_PASS();
    return true;
}

// Performance test: Resize overhead
bool test_perf_resize() {
    TEST_START("Performance: Resize overhead");
    
    hashmap_t *map = hashmap_create(16, NULL, NULL);
    const int N = 100000;
    
    double start = get_time_ms();
    for (int i = 0; i < N; i++) {
        int key = i;
        int value = i;
        hashmap_put(map, &key, sizeof(int), &value);
    }
    double end = get_time_ms();
    
    double time_ms = end - start;
    printf("  Inserted %d entries (with resizing) in %.2f ms\n", N, time_ms);
    printf("  Final size: %zu\n", hashmap_size(map));
    
    hashmap_destroy(map);
    TEST_PASS();
    return true;
}

// Performance test: String keys
bool test_perf_string_keys() {
    TEST_START("Performance: String keys");
    
    hashmap_t *map = hashmap_create(16, NULL, NULL);
    const int N = 10000;
    
    char keys[10000][32];
    for (int i = 0; i < N; i++) {
        snprintf(keys[i], sizeof(keys[i]), "key_%d", i);
    }
    
    double start = get_time_ms();
    for (int i = 0; i < N; i++) {
        hashmap_put(map, keys[i], strlen(keys[i]) + 1, "value");
    }
    double end = get_time_ms();
    
    double time_ms = end - start;
    double ops_per_sec = (N / time_ms) * 1000.0;
    
    printf("  Inserted %d string keys in %.2f ms (%.0f ops/sec)\n", 
           N, time_ms, ops_per_sec);
    
    hashmap_destroy(map);
    TEST_PASS();
    return true;
}

// ============================================================================
// Test Runner
// ============================================================================

int main() {
    printf("========================================\n");
    printf("Hashmap Test Suite\n");
    printf("========================================\n\n");
    
    // API Robustness Tests
    printf("--- API Robustness Tests ---\n");
    test_func_t api_tests[] = {
        test_basic_put_get,
        test_update_key,
        test_remove_key,
        test_contains,
        test_empty_hashmap,
        test_string_keys,
        test_struct_keys,
        test_binary_data,
        test_variable_length_keys,
        test_large_insertions,
        test_collisions,
        test_resize,
        test_clear,
        test_memory_cleanup,
        test_invalid_parameters,
        test_key_size_mismatch,
        test_destroy_null,
        test_default_capacity,
        NULL
    };
    
    for (int i = 0; api_tests[i] != NULL; i++) {
        api_tests[i]();
    }
    
    printf("\n--- Performance Tests ---\n");
    test_func_t perf_tests[] = {
        test_perf_insert,
        test_perf_lookup,
        test_perf_mixed,
        test_perf_resize,
        test_perf_string_keys,
        NULL
    };
    
    for (int i = 0; perf_tests[i] != NULL; i++) {
        perf_tests[i]();
    }
    
    // Summary
    printf("\n========================================\n");
    printf("Test Summary\n");
    printf("========================================\n");
    printf("Tests run:    %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    
    if (tests_failed == 0) {
        printf("\n✓ All tests passed!\n");
        return 0;
    } else {
        printf("\n✗ Some tests failed!\n");
        return 1;
    }
}

