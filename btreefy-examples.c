#include <stdio.h>

#include "btreefy.h"

btf_node_status_t reset_variables_action(btf_tree_st *tree, void *data,
                                         size_t datalen)
{
    printf("Reset variables\n");
    return BTF_FAILURE_STATUS;
}

btf_node_status_t run_ble_action(btf_tree_st *tree, void *data, size_t datalen)
{
    printf("Run BLE\n");
    return BTF_FAILURE_STATUS;
}

btf_node_status_t run_lora_action(btf_tree_st *tree, void *data, size_t datalen)
{
    printf("Run LoRa\n");
    return BTF_FAILURE_STATUS;
}

btf_node_status_t go_to_sleep_action(btf_tree_st *tree, void *data,
                                     size_t datalen)
{
    printf("Go to sleep\n");
    return BTF_SUCCESS_STATUS;
}

struct btf_node nodes[] = {
    [0] = {.parent  = BTF_NULL_NODE,
           .child   = 1,
           .sibling = BTF_NULL_NODE,
           .action  = NULL,
           .control = btf_sequence_policy_fn},
    [1] = {.parent  = 0,
           .child   = 2,
           .sibling = 3,
           .action  = NULL,
           .control = btf_success_policy_fn},
    [2] = {.parent  = 1,
           .child   = BTF_NULL_NODE,
           .sibling = BTF_NULL_NODE,
           .action  = reset_variables_action,
           .control = NULL},
    [3] = {.parent  = 0,
           .child   = 4,
           .sibling = 6,
           .action  = NULL,
           .control = btf_fallback_policy_fn},
    [4] = {.parent  = 3,
           .child   = BTF_NULL_NODE,
           .sibling = 5,
           .action  = run_ble_action,
           .control = NULL},
    [5] = {.parent  = 3,
           .child   = BTF_NULL_NODE,
           .sibling = BTF_NULL_NODE,
           .action  = run_lora_action,
           .control = NULL},
    [6] = {.parent  = 0,
           .child   = 7,
           .sibling = BTF_NULL_NODE,
           .action  = NULL,
           .control = btf_success_policy_fn},
    [7] = {.parent  = 6,
           .child   = BTF_NULL_NODE,
           .sibling = BTF_NULL_NODE,
           .action  = go_to_sleep_action,
           .control = NULL},
};

int main(void)
{
    btf_tree_st tree;

    if (btf_init(&tree, nodes, sizeof(nodes)) == 0)
    {
        btf_tree_controller(&tree);
    }
    return 0;
}