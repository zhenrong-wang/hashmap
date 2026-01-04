# Simple Hashmap Implementation in C

A simple hashmap implementation in C that mimics the functionality of `std::map` or `std::unordered_map` from C++.

## Features

- **Generic key-value storage**: Works with any pointer type
- **Customizable hash and comparison functions**: Default string support included
- **Automatic resizing**: Grows when load factor exceeds 0.75
- **Collision handling**: Uses separate chaining
- **Memory management**: Optional cleanup functions for keys and values
- **Standard operations**: put, get, remove, contains, size, clear

## API

### Creating a Hashmap

```c
hashmap_t *hashmap_create(
    size_t initial_capacity,
    hash_func_t hash_func,        // NULL for default string hash
    key_compare_func_t key_compare, // NULL for default string compare
    key_free_func_t key_free,      // NULL for no cleanup
    value_free_func_t value_free   // NULL for no cleanup
);
```

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

### String Keys (Default)

```c
hashmap_t *map = hashmap_create(16, NULL, NULL, NULL, NULL);
hashmap_put(map, "key1", "value1");
char *value = (char *)hashmap_get(map, "key1");
hashmap_destroy(map);
```

### Integer Keys (Custom Functions)

```c
hashmap_t *map = hashmap_create(16, int_hash, int_compare, NULL, NULL);
int key = 42;
int value = 100;
hashmap_put(map, &key, &value);
int *result = (int *)hashmap_get(map, &key);
hashmap_destroy(map);
```

### With Automatic Memory Cleanup

```c
hashmap_t *map = hashmap_create(16, NULL, NULL, free, free);
char *key = strdup("name");
char *val = strdup("John");
hashmap_put(map, key, val);
// hashmap_destroy will automatically free keys and values
hashmap_destroy(map);
```

## Implementation Details

- Uses separate chaining for collision resolution
- Default capacity: 16 buckets
- Resizes when load factor > 0.75 (doubles capacity)
- Default hash function: djb2 algorithm for strings
- Default comparison: `strcmp` for strings

