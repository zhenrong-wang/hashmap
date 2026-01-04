# Simple Hashmap Implementation in C

A simple, truly generic **header-only** hashmap implementation in C that mimics the functionality of `std::unordered_map` from C++.

**Header-Only**: Just `#include "hashmap.h"` - no separate compilation needed!

## Features

- **Truly generic**: Works with any byte block as keys (int, strings, structs, binary data)
- **Simple API**: No need for custom hash/compare functions - just provide key size
- **Automatic resizing**: Grows when load factor exceeds 0.75
- **Collision handling**: Uses separate chaining
- **Memory management**: Optional cleanup functions for values
- **Safe**: Keys are always copied internally (no dangling pointer issues)
- **Standard operations**: put, get, remove, contains, size, clear

## API

### Creating a Hashmap

```c
hashmap_t *hashmap_create(
    size_t initial_capacity,  // Initial number of buckets (0 defaults to 16)
    key_free_func_t key_free,      // NULL (keys are always copied internally)
    value_free_func_t value_free   // NULL for no cleanup
);
```

**Note**: Keys are always copied and stored with their sizes internally. You don't need to provide hash or comparison functions - the library handles everything generically using byte-wise hashing and comparison.

### Operations

- `bool hashmap_put(hashmap_t *map, const void *key, size_t key_size, void *value)` - Insert or update
- `void *hashmap_get(const hashmap_t *map, const void *key, size_t key_size)` - Get value
- `bool hashmap_remove(hashmap_t *map, const void *key, size_t key_size)` - Remove entry
- `bool hashmap_contains(const hashmap_t *map, const void *key, size_t key_size)` - Check existence
- `size_t hashmap_size(const hashmap_t *map)` - Get size
- `bool hashmap_is_empty(const hashmap_t *map)` - Check if empty
- `void hashmap_clear(hashmap_t *map)` - Clear all entries
- `void hashmap_destroy(hashmap_t *map)` - Destroy hashmap

## Building

Since this is a header-only implementation, you can simply include `hashmap.h` in your code. No separate compilation needed!

```bash
# Build example
make

# Or just compile directly:
gcc -o my_program my_program.c
```

## Running Example

```bash
./hashmap_example
```

## Usage Examples

### Integer Keys

```c
hashmap_t *map = hashmap_create(16, NULL, NULL);
int key = 42;
int value = 100;
hashmap_put(map, &key, sizeof(int), &value);
int *result = (int *)hashmap_get(map, &key, sizeof(int));
hashmap_destroy(map);
```

### String Keys

```c
hashmap_t *map = hashmap_create(16, NULL, NULL);
hashmap_put(map, "apple", strlen("apple") + 1, "red");
char *value = (char *)hashmap_get(map, "apple", strlen("apple") + 1);
hashmap_destroy(map);
```

### Struct Keys

```c
typedef struct { int x; int y; } point_t;
hashmap_t *map = hashmap_create(16, NULL, NULL);
point_t p = {1, 2};
hashmap_put(map, &p, sizeof(point_t), "origin");
char *label = (char *)hashmap_get(map, &p, sizeof(point_t));
hashmap_destroy(map);
```

### Variable-Length Binary Data

```c
hashmap_t *map = hashmap_create(16, NULL, NULL);
uint8_t key1[] = {0x01, 0x02, 0x03};
uint8_t key2[] = {0x04, 0x05, 0x06, 0x07, 0x08};  // Different length!

hashmap_put(map, key1, sizeof(key1), "value1");
hashmap_put(map, key2, sizeof(key2), "value2");

char *result = (char *)hashmap_get(map, key1, sizeof(key1));
hashmap_destroy(map);
```

### With Memory Cleanup

```c
hashmap_t *map = hashmap_create(16, NULL, free);  // free values on destroy

char *val1 = strdup("John");
char *val2 = strdup("Jane");

hashmap_put(map, "name1", strlen("name1") + 1, val1);
hashmap_put(map, "name2", strlen("name2") + 1, val2);

// hashmap_destroy will automatically free all values
hashmap_destroy(map);
```

## How It Works

The hashmap uses a **truly generic approach**:

1. **Keys are always copied**: When you call `hashmap_put()`, the key data is copied and stored with its size
2. **Byte-wise hashing**: Uses FNV-1a hash algorithm on the raw bytes
3. **Byte-wise comparison**: Uses `memcmp()` for comparison
4. **No type assumptions**: Works with any byte pattern, any length (even with null bytes!)

This is similar to how C++ `std::unordered_map<std::vector<uint8_t>, V>` works - the container knows the size of each key and handles everything generically.

## Comparison with C++

In C++, `std::unordered_map` uses templates and compile-time type information. For example:
- `std::unordered_map<int, V>` - compiler knows `int` size
- `std::unordered_map<std::string, V>` - `std::string` knows its own size
- `std::unordered_map<std::vector<uint8_t>, V>` - `std::vector` knows its size

In C, we achieve similar genericity by:
- Requiring `key_size` parameter at runtime (instead of compile-time type info)
- Copying keys and storing sizes (like `std::vector<uint8_t>` does)
- Using byte-wise hashing/comparison (works for any type)

## Implementation Details

- Uses separate chaining for collision resolution
- Default capacity: 16 buckets
- Resizes when load factor > 0.75 (doubles capacity)
- Hash algorithm: FNV-1a
- Keys are always copied (safe, but uses more memory)
- No hash/compare functions needed - everything is generic!

## Trade-offs

**Pros:**
- ✅ Truly generic - works with any byte block
- ✅ Simple API - no custom functions needed
- ✅ Safe - keys are copied (no dangling pointers)
- ✅ Consistent - same API for all key types

**Cons:**
- ⚠️ Always copies keys (memory overhead)
- ⚠️ Requires `key_size` parameter at each operation
- ⚠️ Slightly more memory per entry (stores key_size)

For most use cases, the simplicity and safety outweigh the memory overhead.
