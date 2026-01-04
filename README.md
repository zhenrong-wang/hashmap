# Simple Hashmap Implementation in C

A simple hashmap implementation in C that mimics the functionality of `std::map` or `std::unordered_map` from C++.

## Features

- **Generic key-value storage**: Works with any pointer type (keys can be binary data, not just strings)
- **Required hash and comparison functions**: Ensures type safety - no assumptions about key format
- **Helper functions provided**: For strings, integers, and pointers
- **Automatic resizing**: Grows when load factor exceeds 0.75
- **Collision handling**: Uses separate chaining
- **Memory management**: Optional cleanup functions for keys and values
- **Standard operations**: put, get, remove, contains, size, clear

## API

### Creating a Hashmap

```c
hashmap_t *hashmap_create(
    size_t initial_capacity,
    hash_func_t hash_func,        // REQUIRED - function to hash keys
    key_compare_func_t key_compare, // REQUIRED - function to compare keys
    key_free_func_t key_free,      // NULL for no cleanup
    value_free_func_t value_free   // NULL for no cleanup
);
```

**Note**: `hash_func` and `key_compare` are **required** and cannot be NULL. This ensures the hashmap works correctly with any key type, including binary data. Use the helper functions below for common cases.

### Helper Functions

The library provides helper functions for common key types:

**Hash functions:**
- `hashmap_string_hash()` - For null-terminated C strings
- `hashmap_int_hash()` - For integer keys (pass pointer to int)
- `hashmap_ptr_hash()` - For pointer keys

**Comparison functions:**
- `hashmap_string_compare()` - For null-terminated C strings
- `hashmap_int_compare()` - For integer keys
- `hashmap_ptr_compare()` - For pointer keys

For binary data or custom types, you must provide your own hash and comparison functions.

### Operations

- `bool hashmap_put(hashmap_t *map, void *key, void *value)` - Insert or update
- `void *hashmap_get(const hashmap_t *map, const void *key)` - Get value
- `bool hashmap_remove(hashmap_t *map, const void *key)` - Remove entry
- `bool hashmap_contains(const hashmap_t *map, const void *key)` - Check existence
- `size_t hashmap_size(const hashmap_t *map)` - Get size
- `bool hashmap_is_empty(const hashmap_t *map)` - Check if empty
- `void hashmap_clear(hashmap_t *map)` - Clear all entries
- `void hashmap_destroy(hashmap_t *map)` - Destroy hashmap

## Building

```bash
make
```

## Running Example

```bash
./hashmap_example
```

## Usage Examples

### String Keys (Using Helper Functions)

```c
hashmap_t *map = hashmap_create(16, hashmap_string_hash, hashmap_string_compare, NULL, NULL);
hashmap_put(map, "key1", "value1");
char *value = (char *)hashmap_get(map, "key1");
hashmap_destroy(map);
```

### Integer Keys (Using Helper Functions)

```c
hashmap_t *map = hashmap_create(16, hashmap_int_hash, hashmap_int_compare, NULL, NULL);
int key = 42;
int value = 100;
hashmap_put(map, &key, &value);
int *result = (int *)hashmap_get(map, &key);
hashmap_destroy(map);
```

### With Automatic Memory Cleanup

```c
hashmap_t *map = hashmap_create(16, hashmap_string_hash, hashmap_string_compare, free, free);
char *key = strdup("name");
char *val = strdup("John");
hashmap_put(map, key, val);
// hashmap_destroy will automatically free keys and values
hashmap_destroy(map);
```

### Binary Data Keys (Custom Functions)

For binary data, you must provide custom hash and comparison functions:

```c
// Hash function for binary data of known size
size_t binary_hash(const void *key) {
    const uint8_t *data = (const uint8_t *)key;
    size_t hash = 5381;
    // Hash first 16 bytes (adjust as needed)
    for (int i = 0; i < 16; i++) {
        hash = ((hash << 5) + hash) + data[i];
    }
    return hash;
}

int binary_compare(const void *key1, const void *key2) {
    return memcmp(key1, key2, 16); // Compare first 16 bytes
}

hashmap_t *map = hashmap_create(16, binary_hash, binary_compare, NULL, NULL);
uint8_t key[16] = {0x01, 0x02, ...};
hashmap_put(map, key, value);
hashmap_destroy(map);
```

## Implementation Details

- Uses separate chaining for collision resolution
- Default capacity: 16 buckets
- Resizes when load factor > 0.75 (doubles capacity)
- Hash and comparison functions are **required** - no default assumptions about key format
- Helper functions provided for strings (djb2 hash), integers, and pointers
- Supports any key type including binary data - just provide appropriate hash/compare functions

