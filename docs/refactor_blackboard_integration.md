# Plan: Refactor Blackboard Integration

This document outlines the plan to refactor the blackboard integration within the BTreeFy library. The goal is to improve encapsulation, simplify APIs, and create a more robust and scalable design by tightly coupling a behavior tree with its blackboard context.

## 1. Modify Core Tree Structure

**File:** `include/btreefy/btreefy_objs.h`

The `btf_tree_st` struct will be updated to include a pointer to its associated blackboard.

```c
typedef struct
{
    struct btf_node         *nodes;
    uint32_t                 size;
    uintptr_t                running_node_index;
    struct btf_blackboard   *blackboard; // Add this field
} btf_tree_st;
```

## 2. Update Tree Initialization

**Files:** `include/btreefy/btreefy.h`, `src/btreefy.c`

The `btf_init` function will be modified to accept a blackboard pointer and link it to the tree instance during initialization.

-   **`btreefy.h`:** Update function signature.
    ```c
    int32_t btf_init(btf_tree_st *tree, struct btf_node *nodes, uint32_t tree_size, struct btf_blackboard *blackboard);
    ```
-   **`btreefy.c`:** Implement the logic to store the blackboard.
    ```c
    int32_t btf_init(btf_tree_st *tree, /* ... */, struct btf_blackboard *blackboard)
    {
        // ... existing initialization logic ...
        tree->blackboard = blackboard;
        return BTF_ERROR_OK;
    }
    ```

## 3. Create a Blackboard Reference Helper Macro

**File:** `include/btreefy/btf_blackboard.h`

To solve the problem of getting the address of the auto-generated blackboard variable, a helper macro will be created.

```c
#define BTF_BLACKBOARD_GET(name) &BTF_UTILS_CONCAT(_btf_blackboard_obj_, name)
```

## 4. Refactor the Tree Runner

**Files:** `include/btreefy/btf_tree_runner.h`, `src/runners/btf_tree_runner_posix.c`, `src/runners/btf_tree_runner_zephyr.c`

The `btf_runner_init` function will be simplified. It will no longer take a blackboard as a direct parameter, but will instead retrieve it from the `btf_tree_st` object. This centralizes the tree's context.

-   **`btf_tree_runner.h`:** Update function signature.
    ```c
    int32_t btf_runner_init(struct btf_runner *runner, btf_tree_st *tree, struct btf_runner_config *config);
    ```
-   **`btf_tree_runner_posix.c`:** Update implementation to get blackboard from the tree.
    ```c
    int32_t btf_runner_init(struct btf_runner *runner, btf_tree_st *tree, struct btf_runner_config *config)
    {
        // ...
        runner->tree = tree;
        if (tree->blackboard != NULL) {
            btf_blackboard_set_notify_cb(tree->blackboard, blackboard_to_runner_notify_cb, runner);
        }
        // ...
    }
    ```
-   The same change will be applied to the Zephyr implementation.

## 5. Update Example Application

**File:** `examples/pc/btreefy-examples.c`

The `main` function in the example will be updated to use the new, cleaner initialization flow.

```c
int main(void)
{
    // ...
    if (btf_init(&tree, nodes, nodes_size, BTF_BLACKBOARD_GET(app_blackboard)) != 0)
    {
        printf("Failed to init tree
");
        return -1;
    }

    if (btf_runner_init(&runner, &tree, &runner_config)) // No longer needs blackboard
    {
        printf("Failed to init runner
");
        return -1;
    }
    // ...
}
```

## 6. (Optional) Refactor Node Actions

**File:** `examples/pc/btreefy-examples.c`

As a final step to demonstrate the new architecture, node actions and conditions can be refactored to use the blackboard from the tree pointer they receive, instead of relying on global macros.

-   **Before:**
    ```c
    btf_node_status_t open_door_request_cond(btf_tree_st *tree, void *data, size_t datalen)
    {
        // ...
        BTF_BLACKBOARD_RETRIEVE_DATA(app_blackboard, open_request, flag_occurred);
        // ...
    }
    ```
-   **After:**
    ```c
    btf_node_status_t open_door_request_cond(btf_tree_st *tree, void *data, size_t datalen)
    {
        // ...
        btf_blackboard_retrieve_data(tree->blackboard, offsetof(struct app_blackboard, open_request), &flag_occurred, sizeof(flag_occurred));
        // ...
    }
    ```
This final step makes the action/condition functions more generic and reusable, as they are no longer tied to a specific blackboard name.
