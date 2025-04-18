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
           .p_fn = (void (*)(void))btf_sequence_policy_fn},
    [1] = {.parent  = 0,
           .child   = 2,
           .sibling = 3,
           .p_fn = (void (*)(void))btf_success_policy_fn},
    [2] = {.parent  = 1,
           .child   = BTF_NULL_NODE,
           .sibling = BTF_NULL_NODE,
           .p_fn  =(void (*)(void)) reset_variables_action},
    [3] = {.parent  = 0,
           .child   = 4,
           .sibling = 6,
           .p_fn = (void (*)(void))btf_fallback_policy_fn},
    [4] = {.parent  = 3,
           .child   = BTF_NULL_NODE,
           .sibling = 5,
           .p_fn  =(void (*)(void)) run_ble_action},
    [5] = {.parent  = 3,
           .child   = BTF_NULL_NODE,
           .sibling = BTF_NULL_NODE,
           .p_fn  =(void (*)(void)) run_lora_action},
    [6] = {.parent  = 0,
           .child   = 7,
           .sibling = BTF_NULL_NODE,
           .p_fn = (void (*)(void))btf_success_policy_fn},
    [7] = {.parent  = 6,
           .child   = BTF_NULL_NODE,
           .sibling = BTF_NULL_NODE,
           .p_fn  =(void (*)(void)) go_to_sleep_action},
};

int main(void)
{
    btf_tree_st tree;

    if (btf_init(&tree, nodes, sizeof(nodes)) == 0)
    {
        btf_tick_tree(&tree);
    }
    return 0;
}