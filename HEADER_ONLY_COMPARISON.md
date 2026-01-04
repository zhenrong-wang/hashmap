# Header-Only vs Separate Compilation: Key Differences

## Overview

The hashmap implementation is **functionally identical** in both versions - same algorithms, same data structures, same API. The difference is purely in **how the code is organized and compiled**.

## Previous Implementation (.h + .c)

### Structure
```
hashmap.h  - Function declarations (interfaces)
hashmap.c  - Function definitions (implementations)
```

### Compilation Process
```bash
# Step 1: Compile hashmap.c to object file
gcc -c hashmap.c -o hashmap.o

# Step 2: Compile your program
gcc -c my_program.c -o my_program.o

# Step 3: Link everything together
gcc hashmap.o my_program.o -o my_program
```

### Code Organization

**hashmap.h** (declarations only):
```c
// Forward declaration
typedef struct hashmap hashmap_t;

// Function declarations (no body)
hashmap_t *hashmap_create(...);
void hashmap_destroy(hashmap_t *map);
bool hashmap_put(...);
```

**hashmap.c** (implementations):
```c
#include "hashmap.h"

// Structure definitions
struct hashmap { ... };
typedef struct entry { ... } entry_t;

// Function definitions (with body)
hashmap_t *hashmap_create(...) {
    // implementation
}

static size_t generic_hash(...) {  // static = internal only
    // implementation
}
```

### Characteristics

1. **Separate Compilation Units**
   - `hashmap.c` compiled once into `hashmap.o`
   - Object file linked into final executable

2. **Function Calls**
   - Functions are separate entities
   - Calls go through function pointers (unless LTO)
   - Each function has one definition in the binary

3. **Linking Required**
   - Must link `hashmap.o` with your program
   - Makefile needs to compile both files

4. **Code Visibility**
   - Internal functions (`static`) hidden from other files
   - Only public API visible in header

---

## Current Implementation (Header-Only)

### Structure
```
hashmap.h  - Everything (declarations + implementations)
```

### Compilation Process
```bash
# Just compile your program - hashmap code included automatically
gcc my_program.c -o my_program
```

### Code Organization

**hashmap.h** (everything):
```c
// Structure definitions
typedef struct hashmap { ... } hashmap_t;
typedef struct hashmap_entry { ... } hashmap_entry_t;

// All functions with static inline
static inline hashmap_t *hashmap_create(...) {
    // implementation
}

static inline size_t hashmap_generic_hash(...) {
    // implementation
}
```

### Characteristics

1. **Inline Compilation**
   - Code compiled into each `.c` file that includes the header
   - Each translation unit gets its own copy of the code

2. **Function Inlining**
   - `static inline` tells compiler: "inline this if possible"
   - Compiler can inline functions for better performance
   - No function call overhead (when inlined)

3. **No Linking Needed**
   - Just include the header
   - No separate object file to link

4. **Code Duplication (at compile time)**
   - Each `.c` file that includes the header compiles the functions
   - But `static inline` + optimization = no duplication in final binary

---

## Key Differences Summary

| Aspect | Previous (.h + .c) | Current (Header-Only) |
|--------|-------------------|---------------------|
| **Files** | 2 files (hashmap.h, hashmap.c) | 1 file (hashmap.h) |
| **Compilation** | Compile .c separately, then link | Included automatically |
| **Build Process** | `gcc hashmap.c my_program.c` | `gcc my_program.c` |
| **Function Storage** | One copy in binary | Inlined (no separate copy) |
| **Performance** | Function calls | Inlined (potentially faster) |
| **Code Size** | Smaller (one copy) | Same (compiler optimizes) |
| **Distribution** | Need .h + .c (or .o/.a) | Just .h file |
| **Visibility** | Internal functions hidden | All code visible |

---

## Are They Essentially the Same?

### ✅ **Functionally: YES**
- Same algorithms (FNV-1a hash, separate chaining)
- Same data structures
- Same API
- Same behavior
- Same performance characteristics (or better with inlining)

### ⚠️ **Implementation: NO**

**Previous (.h + .c):**
```c
// hashmap.h
hashmap_t *hashmap_create(...);  // Declaration

// hashmap.c  
hashmap_t *hashmap_create(...) {  // Definition
    // code
}
```

**Current (Header-Only):**
```c
// hashmap.h
static inline hashmap_t *hashmap_create(...) {  // Definition with inline hint
    // code
}
```

### The `static inline` Keyword

**`static`**: 
- Function is local to the translation unit
- No name collision if included in multiple files
- Each `.c` file gets its own copy (at compile time)

**`inline`**:
- Hint to compiler: "try to inline this function"
- Compiler may inline calls (eliminate function call overhead)
- If not inlined, creates a local copy (due to `static`)

**Result**:
- No multiple definition errors (thanks to `static`)
- Better optimization potential (thanks to `inline`)
- Same or better performance

---

## Performance Comparison

### Previous Implementation
```assembly
call hashmap_get    ; Function call (indirect jump)
```

### Header-Only Implementation (with optimization)
```assembly
; Code inlined directly - no function call!
mov  rax, [map+buckets]
; ... hash calculation inlined ...
; ... comparison inlined ...
```

**With `-O2` or `-O3` optimization**, the header-only version can be faster because:
- Functions can be inlined
- No function call overhead
- Better optimization across function boundaries

---

## When to Use Each?

### Header-Only (Current) ✅
- **Small to medium libraries** (like this hashmap)
- **Template-like functionality** (generic code)
- **Performance-critical code** (inlining helps)
- **Easy distribution** (single file)
- **No build complexity** (just include)

### Separate Compilation (.h + .c)
- **Large libraries** (avoid recompiling everything)
- **Binary distribution** (ship .o/.a files, hide source)
- **Faster incremental builds** (only recompile changed .c files)
- **Link-time optimization** (LTO can achieve similar results)

---

## Real-World Example

### Previous Implementation
```bash
$ ls
hashmap.h  hashmap.c  my_program.c

$ gcc -c hashmap.c        # Compile library
$ gcc -c my_program.c      # Compile program  
$ gcc hashmap.o my_program.o -o my_program  # Link

$ nm my_program | grep hashmap
0000000000401234 T hashmap_create
0000000000401456 T hashmap_get
# Functions exist as separate symbols
```

### Header-Only Implementation
```bash
$ ls
hashmap.h  my_program.c

$ gcc -O2 my_program.c -o my_program  # One step!

$ nm my_program | grep hashmap
# No symbols (functions inlined, no separate copies)
```

---

## Code Size Impact

**Myth**: Header-only creates larger binaries

**Reality**: With modern compilers and optimization:
- `static inline` functions are inlined
- Compiler deduplicates identical code
- Final binary size is **the same or smaller**
- Only difference: compilation time (header-only recompiles code in each file)

---

## Conclusion

**They are functionally the same**, but:

1. **Header-only is simpler**: Just include, no linking
2. **Header-only can be faster**: Better inlining opportunities
3. **Header-only is easier to distribute**: Single file
4. **Previous was better for large codebases**: Faster incremental builds

For a small, generic library like this hashmap, **header-only is the better choice** because:
- ✅ Simpler to use
- ✅ Better performance potential
- ✅ Easier distribution
- ✅ No build complexity

The trade-off (slightly longer compile times) is negligible for most use cases.

