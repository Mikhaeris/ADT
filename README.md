# ADT - Abstract Data Types

Simple implementations of basic abstract data types in C. Each data structure is a single self-contained header, paired with its own unit-test suite.

## TODO

- [x] **Dynamic array (vector)**
- [ ] **Stack**
- [ ] **Doubly linked list**
- [ ] **Queue**
- [ ] **Deque**
- [ ] **Map**
- [ ] **Set**
- [ ] **Priority queue**
- [ ] **BST**
- [ ] **Ordered Set**
- [ ] **Ordered Map**
- [ ] **Trie**
- [ ] **Skip list**

## Layout

```
.
├── vector.h          # the data structures (header-only)
└── tests/
    ├── Makefile      # build + run the test suites
    ├── vector.c      # tests for vector.h
    └── lib/
        └── greatest.h
```

## Dynamic array (vector)

A growable, type-generic array. The handle is an ordinary typed pointer (`T *`) pointing straight at the elements, with the bookkeeping (size, capacity, element size) tucked into a header that sits just before the data. So you index and iterate it exactly like a raw array — `v[i]`, `&v[i]`, `for (size_t i = 0; i < vec_size(v); i++)` — while the macros keep the metadata in sync.

Capacity grows geometrically (doubling), so a run of appends is amortized O(1).

### Usage

`vector.h` is header-only. Define `VECTOR_IMPL_CENTRY` in **exactly one** translation unit before including it; include it normally everywhere else.

```c
#define VECTOR_IMPL_SENTRY   /* in one .c file only */
#include "vector.h"

#include <stdio.h>

int main(void) {
    int *v = NULL;           /* must start as NULL */
    vec_init(v, int);

    vec_push_back(v, 10);
    vec_push_back(v, 20);
    vec_push_back(v, 30);    /* [10, 20, 30]     */

    vec_insert(v, 1, 15);    /* [10, 15, 20, 30] */
    vec_erase(v, 0);         /* [15, 20, 30]     */

    for (size_t i = 0; i < vec_size(v); i++)
        printf("%d\n", v[i]);

    vec_free(v);             /* frees and sets v = NULL */
    return 0;
}
```

Elements are stored by value, so any type works, including structs:

```c
typedef struct { int id; double score; } record;

record *rs = NULL;
vec_init(rs, record);

record r = { .id = 1, .score = 9.5 };
vec_push_back(rs, r);

printf("%d %.1f\n", rs[0].id, rs[0].score);
vec_free(rs);
```

### API

| Macro | Effect | Time |
| --- | --- | --- |
| `vec_init(v, T)` | Initialize an empty vector with element type `T` | O(1) |
| `vec_free(v)` | Release storage, set `v` to `NULL` | O(1) |
| `vec_push_back(v, x)` | Append `x` | amortized O(1) |
| `vec_pop_back(v)` | Remove the last element | O(1) |
| `vec_insert(v, pos, x)` | Insert `x` at index `pos` (`0..size`) | O(n) |
| `vec_erase(v, pos)` | Remove the element at index `pos` | O(n) |
| `vec_clear(v)` | Set size to 0, keep the allocation | O(1) |
| `vec_resize(v, n)` | Resize to `n` elements; any new slots are zeroed | O(n) |
| `vec_swap(a, b)` | Swap the contents of two vectors | O(1) |
| `vec_size(v)` | Number of elements | O(1) |
| `vec_capacity(v)` | Allocated capacity, in elements | O(1) |
| `vec_type_size(v)` | Size of one element, in bytes | O(1) |

### Notes

- Initialize the handle to `NULL` before `vec_init` — a non-`NULL` pointer is treated as already-initialized and left untouched.
- Operations that can grow the vector (`vec_push_back`, `vec_insert`, `vec_resize`) may move the storage on reallocation, invalidating any saved element pointers (`&v[i]`). The handle `v` itself is updated for you; just don't cache raw element pointers across such calls.
- `vec_clear` keeps the buffer for cheap reuse; only `vec_free` returns it to the allocator.
- `vec_push_back` and `vec_insert` are function-like macros, so a braced initializer passed directly is split on its commas by the preprocessor. Assign it to a temporary first (as in the struct example), or wrap the literal in an extra pair of parentheses: `vec_push_back(rs, ((record){ 1, 9.5 }))`.
- The macros use `__typeof__`, so a GCC- or Clang-compatible compiler is required.
- Building requires C11: the implementation uses `_Alignof`/`max_align_t` to keep the elements correctly aligned behind the header.

### Tests

Tests live in `tests/`, one file per data structure, built on the [greatest](https://github.com/silentbicycle/greatest) single-header framework. From `tests/`:

```sh
make            # build and run every suite
make vector     # build and run a single suite
make clean      # remove build artifacts
```

Test binaries are compiled with AddressSanitizer + UndefinedBehaviorSanitizer on by default, so memory errors, leaks, and UB fail the run. Use `make SANITIZE=` for a plain build.
