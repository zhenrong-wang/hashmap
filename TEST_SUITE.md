# Hashmap Test Suite Documentation

## Overview

The test suite provides comprehensive testing for both API robustness and performance characteristics of the hashmap implementation.

## Running Tests

```bash
# Run all tests
make test

# Or directly
make -f Makefile.test test
```

## Test Categories

### 1. API Robustness Tests (18 tests)

These tests verify correct behavior of all API functions:

#### Basic Operations
- **test_basic_put_get**: Basic insertion and retrieval
- **test_update_key**: Updating existing keys
- **test_remove_key**: Removing keys
- **test_contains**: Checking key existence

#### Edge Cases
- **test_empty_hashmap**: Operations on empty hashmap
- **test_invalid_parameters**: NULL pointers, zero sizes
- **test_key_size_mismatch**: Wrong key sizes
- **test_destroy_null**: Destroying NULL map (should not crash)
- **test_default_capacity**: Default capacity when 0 is passed

#### Data Types
- **test_string_keys**: Null-terminated strings
- **test_struct_keys**: Structure types as keys
- **test_binary_data**: Binary data with null bytes
- **test_variable_length_keys**: Keys of different lengths

#### Scalability
- **test_large_insertions**: 10,000 entries
- **test_collisions**: 100 entries with small capacity (forces collisions)
- **test_resize**: Resize behavior verification
- **test_clear**: Clear operation

#### Memory Management
- **test_memory_cleanup**: Value cleanup with `value_free`

### 2. Performance Tests (5 tests)

These tests measure performance characteristics:

#### **test_perf_insert**
- **Operation**: Insert 100,000 integer entries
- **Metrics**: Time, operations per second
- **Expected**: > 1M ops/sec

#### **test_perf_lookup**
- **Operation**: 1,000,000 lookups on 100,000 entries
- **Metrics**: Time, operations per second, hit rate
- **Expected**: > 10M ops/sec

#### **test_perf_mixed**
- **Operation**: 200,000 mixed operations (33% insert, 33% lookup, 33% remove)
- **Metrics**: Time, operations per second
- **Expected**: > 5M ops/sec

#### **test_perf_resize**
- **Operation**: Insert 100,000 entries (triggers multiple resizes)
- **Metrics**: Time, final size
- **Expected**: Handles resizing efficiently

#### **test_perf_string_keys**
- **Operation**: Insert 10,000 string keys
- **Metrics**: Time, operations per second
- **Expected**: > 1M ops/sec

## Test Framework

### Test Macros

```c
TEST_START(name)      // Start a test
TEST_PASS()           // Mark test as passed
TEST_FAIL(msg)        // Mark test as failed with message
ASSERT(cond, msg)     // Assert condition, fail if false
```

### Test Structure

Each test function:
1. Returns `bool` (true = pass, false = fail)
2. Uses `TEST_START()` to begin
3. Uses `ASSERT()` for checks
4. Calls `TEST_PASS()` on success
5. Cleans up resources

### Example Test

```c
bool test_basic_put_get() {
    TEST_START("Basic put and get");
    
    hashmap_t *map = hashmap_create(16, NULL, NULL);
    ASSERT(map != NULL, "hashmap_create failed");
    
    int key = 42;
    int value = 100;
    
    ASSERT(hashmap_put(map, &key, sizeof(int), &value), "hashmap_put failed");
    
    int *result = (int *)hashmap_get(map, &key, sizeof(int));
    ASSERT(result != NULL, "hashmap_get returned NULL");
    ASSERT(*result == 100, "Value mismatch");
    
    hashmap_destroy(map);
    TEST_PASS();
    return true;
}
```

## Performance Benchmarks

Typical performance on modern hardware:

| Test | Operations | Time (ms) | Ops/sec |
|------|------------|-----------|---------|
| Insert | 100,000 | ~18 | ~5.5M |
| Lookup | 1,000,000 | ~16 | ~60M |
| Mixed | 200,000 | ~6 | ~30M |
| String Insert | 10,000 | ~0.7 | ~14M |

**Note**: Performance varies based on:
- CPU architecture
- Compiler optimizations
- System load
- Key sizes

## Memory Testing

For memory leak detection, use valgrind:

```bash
valgrind --leak-check=full ./test_hashmap
```

The test suite includes a memory cleanup test, but valgrind provides more comprehensive analysis.

## Coverage

### API Coverage
- ✅ All public functions tested
- ✅ All error paths tested
- ✅ Edge cases covered
- ✅ Memory management verified

### Scenario Coverage
- ✅ Small datasets (< 100 entries)
- ✅ Large datasets (10,000+ entries)
- ✅ Collision scenarios
- ✅ Resize scenarios
- ✅ Different key types
- ✅ Variable-length keys

## Adding New Tests

1. Create a test function returning `bool`
2. Use `TEST_START()` with descriptive name
3. Use `ASSERT()` for all checks
4. Call `TEST_PASS()` on success
5. Add to appropriate test array in `main()`

```c
bool test_new_feature() {
    TEST_START("New feature test");
    
    // Test code here
    ASSERT(condition, "Error message");
    
    TEST_PASS();
    return true;
}
```

## Continuous Integration

The test suite is designed to:
- Exit with code 0 on success
- Exit with code 1 on failure
- Provide clear output for CI systems
- Run quickly (< 1 second typically)

## Known Limitations

1. **No iteration tests**: Iterator API not yet implemented
2. **No concurrent tests**: Thread safety not tested
3. **No stress tests**: Extreme memory pressure scenarios
4. **No fuzzing**: Random input generation not included

## Future Enhancements

- Property-based testing (QuickCheck-style)
- Fuzzing with AFL or similar
- Thread safety tests
- Memory pressure tests
- Performance regression detection
- Coverage analysis integration

