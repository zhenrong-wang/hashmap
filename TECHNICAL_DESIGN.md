# Hashmap Implementation - Technical Design Document

## Executive Summary

This document describes the technical design and implementation of a generic hashmap data structure in C. The implementation provides a truly generic key-value store that works with any byte block as keys, similar to C++'s `std::unordered_map`, but using runtime size parameters instead of compile-time templates.

**Version**: 1.0  
**Language**: C11  
**License**: MIT  
**Lines of Code**: ~270 (implementation) + ~60 (header)

---

## 1. Architecture Overview

### 1.1 Design Philosophy

The hashmap is designed with the following principles:

1. **Truly Generic**: Works with any byte block as keys (int, string, struct, binary data)
2. **Simple API**: No custom hash/compare functions required
3. **Memory Safe**: Keys are always copied internally (no dangling pointer issues)
4. **Consistent**: Same API pattern for all key types
5. **Efficient**: O(1) average case operations with automatic resizing

### 1.2 High-Level Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    hashmap_t                            │
│  ┌──────────────┐  ┌──────────┐  ┌─────────────────┐ │
│  │   buckets[]  │  │ capacity │  │      size       │ │
│  │  (entry_t**) │  │  (size)  │  │     (size)      │ │
│  └──────────────┘  └──────────┘  └─────────────────┘ │
└─────────────────────────────────────────────────────────┘
         │
         ├───> [bucket 0] ──> entry_t ──> entry_t ──> NULL
         ├───> [bucket 1] ──> entry_t ──> NULL
         ├───> [bucket 2] ──> NULL
         └───> [bucket N] ──> entry_t ──> NULL

Each entry_t contains:
  - key (copied data)
  - key_size
  - value (pointer)
  - next (for chaining)
```

---

## 2. Data Structures

### 2.1 Entry Structure

```c
typedef struct entry {
    void *key;          // Copied key data (malloc'd)
    size_t key_size;    // Size of key in bytes
    void *value;        // Value pointer (not copied)
    struct entry *next; // For collision chaining
} entry_t;
```

**Key Design Decisions:**
- **Keys are copied**: Ensures memory safety and allows variable-length keys
- **Values are pointers**: Stored as-is (not copied) for efficiency
- **Linked list chaining**: Handles hash collisions

**Memory Layout:**
```
entry_t: [key*][key_size][value*][next*]
         8B     8B        8B       8B    = 32 bytes (on 64-bit)
         + key data (key_size bytes)
```

### 2.2 Hashmap Structure

```c
struct hashmap {
    entry_t **buckets;        // Array of bucket pointers
    size_t capacity;          // Number of buckets
    size_t size;              // Number of entries
    key_free_func_t key_free; // Not used (keys always freed internally)
    value_free_func_t value_free; // Optional value cleanup
};
```

**Memory Layout:**
```
hashmap_t: [buckets*][capacity][size][key_free*][value_free*]
           8B        8B        8B    8B          8B = 40 bytes
           + buckets array (capacity * 8 bytes)
```

---

## 3. Core Algorithms

### 3.1 Hash Function: FNV-1a

**Algorithm**: Fowler-Noll-Vo hash (FNV-1a variant)

```c
hash = 2166136261UL  // FNV offset basis
for each byte in key:
    hash = hash XOR byte
    hash = hash * 16777619UL  // FNV prime
return hash
```

**Properties:**
- **Fast**: Single multiplication per byte
- **Good distribution**: Well-tested hash function
- **Generic**: Works on any byte sequence
- **Deterministic**: Same input always produces same hash

**Time Complexity**: O(k) where k is key_size

**Why FNV-1a?**
- Simple and fast
- Good avalanche effect (small input changes cause large hash changes)
- Widely used and tested
- No cryptographic requirements (just needs good distribution)

### 3.2 Collision Resolution: Separate Chaining

**Method**: Linked list chaining

```
Bucket[i] ──> entry1 ──> entry2 ──> entry3 ──> NULL
```

**Advantages:**
- Simple implementation
- No clustering issues
- Handles arbitrary load factors
- Easy to resize

**Disadvantages:**
- Extra memory overhead (pointer per entry)
- Cache locality not optimal (entries scattered)

**Alternative Considered**: Open addressing (linear probing)
- **Rejected**: More complex, requires tombstones, worse performance with high load

### 3.3 Resizing Strategy

**Trigger**: Load factor > 0.75 (size >= capacity * 3/4)

**New Capacity**: `new_capacity = old_capacity * 2`

**Process**:
1. Allocate new bucket array (2x size)
2. Rehash all entries (recalculate bucket indices)
3. Move entries to new buckets
4. Free old bucket array

**Time Complexity**: O(n) where n is number of entries

**Why 0.75 Load Factor?**
- Balance between memory usage and performance
- Standard practice (used by Java HashMap, Python dict)
- Keeps average chain length low

**Why Double Capacity?**
- Maintains power-of-2 capacity (good for modulo)
- Simple and predictable growth
- Amortized O(1) insert cost

---

## 4. API Design

### 4.1 Creation

```c
hashmap_t *hashmap_create(
    size_t initial_capacity,
    key_free_func_t key_free,    // NULL (keys always freed internally)
    value_free_func_t value_free // NULL or cleanup function
);
```

**Design Decisions:**
- **No hash/compare functions**: Everything is generic byte-wise
- **key_free parameter**: Kept for API compatibility but unused (keys always freed)
- **Default capacity**: 16 if 0 is passed

### 4.2 Operations

All operations require `key_size` parameter:

```c
bool hashmap_put(hashmap_t *map, const void *key, size_t key_size, void *value);
void *hashmap_get(const hashmap_t *map, const void *key, size_t key_size);
bool hashmap_remove(hashmap_t *map, const void *key, size_t key_size);
```

**Why require key_size?**
- Enables truly generic operation (works with any byte block)
- No assumptions about key format (can handle null bytes)
- Consistent API across all key types
- Similar to how C++ `std::vector<uint8_t>` works

**Trade-off**: Slightly more verbose API, but much more flexible

---

## 5. Memory Management

### 5.1 Key Storage

**Strategy**: Always copy keys

```c
entry->key = malloc(key_size);
memcpy(entry->key, key, key_size);
```

**Benefits:**
- Memory safe (no dangling pointers)
- Keys can be stack-allocated
- Supports variable-length keys
- Keys can contain null bytes

**Cost:**
- Memory overhead (copy per key)
- Allocation overhead (malloc per key)

### 5.2 Value Storage

**Strategy**: Store pointers as-is (not copied)

**Rationale:**
- Values are typically larger than keys
- Copying would be expensive
- Caller manages value lifetime
- Optional cleanup via `value_free`

### 5.3 Cleanup

**Automatic:**
- Keys are always freed on remove/destroy
- Bucket arrays are freed on destroy

**Optional:**
- Values can be freed via `value_free` callback

**Example:**
```c
hashmap_t *map = hashmap_create(16, NULL, free);
hashmap_put(map, "key", 4, strdup("value"));
hashmap_destroy(map); // Automatically frees all values
```

---

## 6. Performance Characteristics

### 6.1 Time Complexity

| Operation | Average Case | Worst Case |
|-----------|--------------|------------|
| `put`     | O(1)         | O(n)       |
| `get`     | O(1)         | O(n)       |
| `remove`  | O(1)         | O(n)       |
| `contains`| O(1)         | O(n)       |
| `clear`   | O(n)         | O(n)       |
| `destroy` | O(n)         | O(n)       |

**Worst Case**: All keys hash to same bucket (O(n) chain)

**Average Case**: Assuming good hash distribution and load factor < 0.75

### 6.2 Space Complexity

**Per Entry:**
- `entry_t` structure: 32 bytes (64-bit)
- Key data: `key_size` bytes
- **Total**: 32 + `key_size` bytes

**Per Hashmap:**
- `hashmap_t` structure: 40 bytes
- Bucket array: `capacity * 8` bytes (64-bit pointers)
- **Total**: 40 + `capacity * 8` + `n * (32 + key_size)` bytes

**Example**: 1000 entries, average key_size=8, capacity=2048
- Structure: 40 bytes
- Buckets: 16,384 bytes
- Entries: 1000 * (32 + 8) = 40,000 bytes
- **Total**: ~56 KB

### 6.3 Performance Optimizations

**Current:**
- Power-of-2 capacity (fast modulo: `hash % capacity`)
- Load factor threshold (keeps chains short)
- Inline hash/compare (no function pointer overhead)

**Potential Future Optimizations:**
- Cache-friendly bucket layout
- SIMD for hash computation (large keys)
- Lazy rehashing
- Small key optimization (store in entry_t if key_size <= 8)

---

## 7. Comparison with C++ std::unordered_map

### 7.1 Similarities

| Feature | C++ | This Implementation |
|---------|-----|---------------------|
| Generic keys | Templates | Runtime size parameter |
| Hash table | Yes | Yes |
| Collision handling | Implementation-defined | Separate chaining |
| Automatic resizing | Yes | Yes |
| O(1) average case | Yes | Yes |

### 7.2 Differences

| Aspect | C++ | This Implementation |
|--------|-----|---------------------|
| Type safety | Compile-time | Runtime (size parameter) |
| Key storage | Depends on type | Always copied |
| Hash function | `std::hash<T>` specialization | Generic FNV-1a |
| API complexity | Template-based | Simple function calls |

### 7.3 Why This Approach?

**C++ Advantages:**
- Compile-time type checking
- Zero-cost abstractions
- Template specialization for optimization

**This Implementation Advantages:**
- Works in C (no templates)
- Runtime flexibility (can mix key types)
- Simpler mental model
- No template instantiation overhead

---

## 8. Thread Safety

**Current Status**: **Not thread-safe**

**Operations are not atomic:**
- Resizing can cause issues
- Concurrent put/get can corrupt structure
- No locking mechanism

**If thread safety needed:**
- Add mutex to hashmap structure
- Lock on all operations
- Consider read-write locks (multiple readers)

**Alternative**: Use external synchronization (caller manages locks)

---

## 9. Error Handling

### 9.1 Failure Modes

1. **Memory allocation failure**
   - `hashmap_create()`: Returns NULL
   - `hashmap_put()`: Returns false
   - `hashmap_resize()`: Returns false (put fails)

2. **Invalid parameters**
   - NULL map/key: Returns false/NULL
   - key_size == 0: Returns false/NULL

3. **No error recovery**
   - No partial rollback
   - Caller must handle failures

### 9.2 Design Decisions

**Why no exceptions?**
- C doesn't have exceptions
- Return codes are explicit

**Why not abort on error?**
- Allows graceful degradation
- Caller can handle errors appropriately

---

## 10. Testing Considerations

### 10.1 Test Cases

**Basic Operations:**
- Insert, retrieve, remove
- Update existing key
- Non-existent key lookup

**Edge Cases:**
- Empty hashmap
- Single entry
- All keys hash to same bucket
- Very large keys
- Keys with null bytes

**Memory:**
- Memory leaks (valgrind)
- Double free protection
- Large number of entries

**Performance:**
- Load factor behavior
- Resize performance
- Hash distribution

### 10.2 Known Limitations

1. **No iteration API**: Can't enumerate keys/values
2. **No capacity query**: Can't check current capacity
3. **No custom hash**: Can't override hash function
4. **No key iteration**: Can't get all keys

---

## 11. Future Enhancements

### 11.1 Potential Additions

1. **Iterator API**
   ```c
   hashmap_iter_t *hashmap_iter_create(hashmap_t *map);
   bool hashmap_iter_next(hashmap_iter_t *iter, void **key, size_t *key_size, void **value);
   ```

2. **Statistics**
   ```c
   size_t hashmap_capacity(const hashmap_t *map);
   double hashmap_load_factor(const hashmap_t *map);
   ```

3. **Bulk Operations**
   ```c
   bool hashmap_put_all(hashmap_t *dst, const hashmap_t *src);
   ```

4. **Key/Value Iteration**
   ```c
   void hashmap_for_each(hashmap_t *map, void (*callback)(void *key, size_t key_size, void *value));
   ```

### 11.2 Performance Improvements

1. **Small key optimization**: Store keys < 8 bytes inline
2. **SIMD hashing**: Use AVX2 for large keys
3. **Better hash function**: Consider xxHash for speed
4. **Cache optimization**: Group entries for better locality

---

## 12. Conclusion

This hashmap implementation provides a simple, generic, and efficient key-value store for C programs. The design prioritizes:

1. **Simplicity**: No custom functions needed
2. **Genericity**: Works with any byte block
3. **Safety**: Keys are copied (no dangling pointers)
4. **Performance**: O(1) average case operations

The trade-off of requiring `key_size` at each operation is acceptable given the flexibility and simplicity gained. The implementation is suitable for most use cases where a generic hash table is needed in C.

---

## Appendix A: Code Statistics

- **Total Lines**: ~330 (header + implementation + examples)
- **Implementation**: ~270 lines
- **Header**: ~60 lines
- **Examples**: ~110 lines
- **Functions**: 9 public, 2 private
- **Data Structures**: 2 (hashmap_t, entry_t)

## Appendix B: Dependencies

- **Standard Library**: `stdlib.h`, `string.h`, `stdint.h`, `stdbool.h`, `stddef.h`
- **Platform**: C11 compliant
- **No External Dependencies**: Pure C standard library

## Appendix C: References

- FNV Hash: http://www.isthe.com/chongo/tech/comp/fnv/
- Hash Table Design: "Introduction to Algorithms" (CLRS)
- C++ std::unordered_map: https://en.cppreference.com/w/cpp/container/unordered_map

