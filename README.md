# Simple Hashmap Implementation in C

A simple hashmap implementation in C that mimics the functionality of `std::map` or `std::unordered_map` from C++.

## Features

- **Generic key-value storage**: Works with any pointer type (keys can be binary data, not just strings)
- **Two creation modes**:
  - Custom hash/compare functions (full control)
  - Generic key-size mode (no custom functions needed for fixed-size types!)
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

**Note**: `hash_func` and `key_compare` are **required** and cannot be NULL when using `hashmap_create()`. This ensures the hashmap works correctly with any key type, including binary data. 

**Alternative**: For fixed-size key types (int, structs, binary data), you can use `hashmap_create_with_key_size()` instead - no custom functions needed!

### Generic Key-Size Mode (Recommended for Fixed-Size Types)

For fixed-size key types (integers, structs, binary data), you can use `hashmap_create_with_key_size()` which automatically handles hashing and comparison using byte-wise operations. **No custom functions needed!**

```c
hashmap_t *hashmap_create_with_key_size(
    size_t initial_capacity,
    size_t key_size,        // Size of key in bytes (e.g., sizeof(int), sizeof(my_struct))
    key_free_func_t key_free,
    value_free_func_t value_free
);
```

This uses the FNV-1a hash algorithm and `memcmp` for comparison, making it perfect for:
- Integers (`sizeof(int)`)
- Structs (`sizeof(my_struct)`)
- Fixed-size binary data
- Any type where keys have a fixed size

### Helper Functions

**Only for variable-length keys**: The library provides helper functions for null-terminated C strings:

- `hashmap_string_hash()` - Hash function for strings
- `hashmap_string_compare()` - Comparison function for strings

**Why only strings?** The generic `hashmap_create_with_key_size()` method handles all fixed-size types (int, structs, pointers, binary data). Strings are special because they're variable-length (null-terminated), so they need special handling.

For custom types with special requirements, you can provide your own hash and comparison functions using `hashmap_create()`.

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

### Generic Key-Size Mode (Easiest for Fixed-Size Types!)

```c
// For integers - no custom functions needed!
hashmap_t *map = hashmap_create_with_key_size(16, sizeof(int), NULL, NULL);
int key = 42;
int value = 100;
hashmap_put(map, &key, &value);
int *result = (int *)hashmap_get(map, &key);
hashmap_destroy(map);

// For structs - also no custom functions needed!
typedef struct { int x; int y; } point_t;
hashmap_t *point_map = hashmap_create_with_key_size(16, sizeof(point_t), NULL, NULL);
point_t p = {1, 2};
hashmap_put(point_map, &p, "origin");
char *label = (char *)hashmap_get(point_map, &p);
hashmap_destroy(point_map);
```

### String Keys (Using Helper Functions)

```c
hashmap_t *map = hashmap_create(16, hashmap_string_hash, hashmap_string_compare, NULL, NULL);
hashmap_put(map, "key1", "value1");
char *value = (char *)hashmap_get(map, "key1");
hashmap_destroy(map);
```

### Integer Keys

**Use the generic method** - no helper functions needed:

```c
hashmap_t *map = hashmap_create_with_key_size(16, sizeof(int), NULL, NULL);
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

### Binary Data Keys

For fixed-size binary data, use `hashmap_create_with_key_size()`:

```c
// For 16-byte binary keys - no custom functions needed!
hashmap_t *map = hashmap_create_with_key_size(16, 16, NULL, NULL);
uint8_t key[16] = {0x01, 0x02, ...};
hashmap_put(map, key, value);
hashmap_destroy(map);
```

For variable-length binary data or special requirements, provide custom functions:

```c
// Custom hash function for variable-length binary data
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
```

## Implementation Details

- Uses separate chaining for collision resolution
- Default capacity: 16 buckets
- Resizes when load factor > 0.75 (doubles capacity)
- Hash and comparison functions are **required** - no default assumptions about key format
- Helper functions provided for strings (djb2 hash), integers, and pointers
- Supports any key type including binary data - just provide appropriate hash/compare functions

