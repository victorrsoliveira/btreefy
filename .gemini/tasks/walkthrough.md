# Walkthrough: Removed uintptr_t References

This walkthrough summarizes the changes made to replace all occurrences of `uintptr_t` in BTreeFy, resolving inconsistencies and adopting native pointer subtraction.

## Changes Made

### 1. `running_node_index` Type Update
In [btreefy_objs.h](file:///home/victor/Documents/Mestrado-UFAL-PPGI/BTreeFy/btreefy-repo/include/btreefy/btreefy_objs.h#L52), `running_node_index` was changed from `uintptr_t` to `uint32_t` to match other node indices (e.g. `parent`, `child`, `sibling`) and avoid unnecessary memory usage on 64-bit platforms.

### 2. Native Pointer Subtraction
In [btreefy.h](file:///home/victor/Documents/Mestrado-UFAL-PPGI/BTreeFy/btreefy-repo/include/btreefy/btreefy.h#L25-L27), the macro `BTF_NODE_ARRAY_INDEX` was simplified to use native C pointer subtraction `(p_node) - (p_nodes)`. C handles the division by `sizeof(struct btf_node)` automatically, removing the need for `uintptr_t` casts or division.

### 3. Cleanup of Casts and Comments
* **[btreefy.c](file:///home/victor/Documents/Mestrado-UFAL-PPGI/BTreeFy/btreefy-repo/src/btreefy.c#L99-L106):** Removed the commented-out `uintptr_t` manual pointer math block and removed the now redundant `(uint32_t)` cast on `tree->running_node_index`.
* **[btreefy_policies.c](file:///home/victor/Documents/Mestrado-UFAL-PPGI/BTreeFy/btreefy-repo/src/btreefy_policies.c#L30):** Updated sequence and fallback policy print formats from `%lu` to `%u` to reflect the new `uint32_t` type.
* **[test_btreefy_core.c](file:///home/victor/Documents/Mestrado-UFAL-PPGI/BTreeFy/btreefy-repo/tests/test_btreefy_core.c#L76):** Removed the unnecessary `(uint32_t)` cast inside `TEST_ASSERT_EQUAL_UINT32` for `tree.running_node_index`.
* **[BehaviorTree.drawio](file:///home/victor/Documents/Mestrado-UFAL-PPGI/BTreeFy/btreefy-repo/models/BehaviorTree.drawio#L823):** Updated class diagram representation string for `curr_run_node` from `uintptr_t` to `uint32_t`.

---

## Verification Results

### Automated Tests
Ran the standard test script:
```bash
./compile-execute.sh --with-tests
```

**Results:**
* Clean build: `BTreeFy-Src`, `unity`, and `test_btreefy_core` compiled successfully without any warnings or errors.
* Test execution:
  ```
  Test project /home/victor/Documents/Mestrado-UFAL-PPGI/BTreeFy/btreefy-repo/build
      Start 1: btreefy_core_tests
  1/1 Test #1: btreefy_core_tests ...............   Passed    0.00 sec

  100% tests passed, 0 tests failed out of 1
  ```
* The codebase behaves exactly as expected with a simplified type signature.
