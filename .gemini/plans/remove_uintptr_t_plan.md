# Implementation Plan: Remove uintptr_t References

This document outlines the proposal to remove all `uintptr_t` occurrences in the BTreeFy codebase, replacing them with standard, clean C pointer arithmetic and type-consistent `uint32_t` indices.

## Goal Description
BTreeFy is designed for embedded environments where portability, minimal memory footprint, and type safety are critical. Currently, the codebase contains a few references to `uintptr_t`:
1. `running_node_index` in `struct btf_tree` is declared as `uintptr_t`, even though it represents an index in the `nodes` array and is used as such.
2. The `BTF_NODE_ARRAY_INDEX` macro manually performs pointer subtraction by casting to `uintptr_t` and dividing by the struct size.
3. Comments and diagram XML files reference `uintptr_t`.

We propose to:
* Change `running_node_index` to `uint32_t` for type safety, consistency, and portability.
* Clean up `BTF_NODE_ARRAY_INDEX` to use native C pointer subtraction, which automatically handles element-size division.
* Remove `(uint32_t)` casts that were previously necessary when casting from `uintptr_t` to `uint32_t` for assertions or logging.

---

## User Review Required
No breaking changes are expected for the public API or behavior. All modifications are internal to types and macros.

---

## Evaluation: Index-Based Node Referencing

The user proposed using **indices** to reference nodes relative to the `nodes` array stored in the tree. Let's evaluate this approach.

### Current Architecture in BTreeFy
BTreeFy **already** implements index-based node referencing for the tree structure:
* `struct btf_node` represents connections (`parent`, `child`, `sibling`) using `uint32_t` indices rather than pointers.
* `BTF_NULL_NODE` is defined as `0xFFFFFFFF` and is used as the sentinel value (equivalent to `NULL` for pointers).

### Why Index-Based Referencing is a Good Idea
1. **Static Memory Initialization (Embedded First):** Since BTreeFy code-generates tree structures from XML models, representing connections as static array indices (`uint32_t`) allows the entire tree to be defined at compile-time as a read-only static array. Pointers would require run-time relocation or complex initialization.
2. **Memory Footprint:** On 64-bit architectures, `uint32_t` indices use 4 bytes, whereas pointers/`uintptr_t` use 8 bytes. This saves 4 bytes per link (12 bytes per node), reducing the RAM/ROM footprint.
3. **Portability:** Standard freestanding toolchains do not always require `<stdint.h>` to define `uintptr_t` (it is optional in the C standard if the environment has no integer type capable of holding a pointer). In contrast, `uint32_t` and native pointer arithmetic are universally supported.

### Are there situations where this might not work?
1. **Dynamic Resizing/Relocation (Not applicable here):** If the `nodes` array is dynamically resized (e.g., via `realloc`), indices remain valid relative to the new base pointer, whereas raw pointers would be invalidated. Since BTreeFy uses static allocation (embedded-first rule), memory relocation is not an issue anyway, but index referencing is still safer.
2. **Bounds Checking:** Direct pointer dereferencing is fast but can cause segmentation faults if invalid. Index referencing requires knowing the base pointer (`tree->nodes`) and checking that the index is less than `tree->size` to avoid out-of-bounds access. In the current implementation of `btf_tick_tree`, bounds checking on indices is trusted to the tree generator, which is standard practice.

### Conclusion
The existing design of BTreeFy already follows this index-based referencing model for tree links. Extending this to `running_node_index` (changing it from `uintptr_t` to `uint32_t`) makes the code fully consistent and removes the need for `uintptr_t`.

---

## Proposed Changes

We will modify 5 files to remove `uintptr_t` completely:

### [btreefy/include]
#### [MODIFY] [btreefy_objs.h](file:///home/victor/Documents/Mestrado-UFAL-PPGI/BTreeFy/btreefy-repo/include/btreefy/btreefy_objs.h)
Change `running_node_index` type from `uintptr_t` to `uint32_t`.

```diff
 struct btf_tree
 {
     struct btf_node *nodes;
     uint32_t         size;
     void *           data;
     size_t           datalen;
-    uintptr_t        running_node_index;
+    uint32_t         running_node_index;
 };
```

#### [MODIFY] [btreefy.h](file:///home/victor/Documents/Mestrado-UFAL-PPGI/BTreeFy/btreefy-repo/include/btreefy/btreefy.h)
Simplify the `BTF_NODE_ARRAY_INDEX` macro to use native C pointer subtraction.

```diff
-#define BTF_NODE_ARRAY_INDEX(p_nodes, p_node)                     \
-    (uint32_t)((((uintptr_t) (p_node)) - ((uintptr_t) (p_nodes))) \
-               / (uintptr_t) sizeof(struct btf_node))
+#define BTF_NODE_ARRAY_INDEX(p_nodes, p_node) \
+    ((uint32_t)((p_node) - (p_nodes)))
```

### [btreefy/src]
#### [MODIFY] [btreefy.c](file:///home/victor/Documents/Mestrado-UFAL-PPGI/BTreeFy/btreefy-repo/src/btreefy.c)
Clean up commented-out code that references `uintptr_t` and remove unnecessary type casts.

```diff
             if (BTF_RUNNING_STATUS == status)
             {
                 // Store the running node index
                 tree->running_node_index =
                     BTF_NODE_ARRAY_INDEX(tree->nodes, p_node);
-                // (((uintptr_t) p_node) - ((uintptr_t) tree->nodes))
-                // / (uintptr_t) sizeof(struct btf_node);
                 printf("Store RUNNING node [%u]\n",
-                       (uint32_t) tree->running_node_index);
+                       tree->running_node_index);
             }
```

#### [MODIFY] [btreefy_policies.c](file:///home/victor/Documents/Mestrado-UFAL-PPGI/BTreeFy/btreefy-repo/src/btreefy_policies.c)
Update log formats to use `%u` instead of `%lu` for printing the `uint32_t` index.

```diff
-        printf("Must abort RUNNING action [%lu]\n", tree->running_node_index);
+        printf("Must abort RUNNING action [%u]\n", tree->running_node_index);
```

### [btreefy/tests]
#### [MODIFY] [test_btreefy_core.c](file:///home/victor/Documents/Mestrado-UFAL-PPGI/BTreeFy/btreefy-repo/tests/test_btreefy_core.c)
Remove the `(uint32_t)` cast in `TEST_ASSERT_EQUAL_UINT32` since `running_node_index` is now natively `uint32_t`.

```diff
-    TEST_ASSERT_EQUAL_UINT32(BTF_NULL_NODE, (uint32_t) tree.running_node_index);
+    TEST_ASSERT_EQUAL_UINT32(BTF_NULL_NODE, tree.running_node_index);
```

### [btreefy/models]
#### [MODIFY] [BehaviorTree.drawio](file:///home/victor/Documents/Mestrado-UFAL-PPGI/BTreeFy/btreefy-repo/models/BehaviorTree.drawio)
Update the internal documentation string in the drawio file from `uintptr_t` to `uint32_t`.

```diff
-curr_run_node: uintptr_t
+curr_run_node: uint32_t
```

---

## Verification Plan

### Automated Tests
To verify compile-time correctness and test integrity, run:
```bash
./compile-execute.sh --with-tests
```
This script will rebuild the library, compile unit tests, and execute the test runner. We expect all tests to pass successfully.
