/**
 * @file btreefy.c
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-03-19
 *
 */

#include "btreefy/btreefy.h"

#include <stdio.h>

// Global
char *btf_global_action_status_string[] = {[BTF_UNDEF_STATUS]   = "UNDEF",
                                           [BTF_SUCCESS_STATUS] = "SUCCESS",
                                           [BTF_FAILURE_STATUS] = "FAILURE",
                                           [BTF_RUNNING_STATUS] = "RUNNING"};

char *btf_global_control_status_string[] = {
    [BTF_UNDEF_EXECUTION_RESULT]    = "UNDEF",
    [BTF_CONTINUE_EXECUTION_RESULT] = "CONTINUE",
    [BTF_RETURN_EXECUTION_RESULT]   = "RETURN",
    [BTF_PAUSE_EXECUTION_RESULT]    = "PAUSE"};

int32_t btf_init(struct btf_tree *tree, struct btf_node *nodes,
                 size_t tree_size)

{
    if ((tree == NULL) || (nodes == NULL) || (tree_size == 0))
    {
        return BTF_ERROR_EINVAL;
    }

    tree->nodes              = nodes;
    tree->size               = tree_size;
    tree->running_node_index = BTF_NULL_NODE;

#if BTF_DEBUG_PRINTF_ENABLED
    printf("BT initialized!\n");
#endif

    return 0;
}

int32_t btf_set_data(struct btf_tree *tree, void *data, size_t datalen)
{
    if ((tree == NULL) || (data == NULL) || (datalen == 0))
    {
        return BTF_ERROR_EINVAL;
    }

    tree->data    = data;
    tree->datalen = datalen;

    return 0;
}

int32_t btf_tick_tree(struct btf_tree *tree)
{
    btf_node_index_t               node_index = 0;
    struct btf_node               *p_node     = NULL;
    struct btf_node               *p_parent   = NULL;
    enum btf_node_status           status     = BTF_UNDEF_STATUS;
    enum btf_node_execution_result exec_res   = BTF_UNDEF_EXECUTION_RESULT;

    if (tree == NULL)
    {
        return BTF_ERROR_EINVAL;
    }

    // Start from root node
    p_node   = &tree->nodes[0];
    p_parent = &tree->nodes[p_node->parent];

    // Loop until status is defined and pointer to node is root again
    while (!((status != BTF_UNDEF_STATUS) && (p_node->parent == BTF_NULL_NODE)))
    {
        // Look for leaf node using DFS approach
        if (BTF_UNDEF_STATUS == status)
        {
            // Leaf node has been reached
            if (p_node->child == BTF_NULL_NODE)
            {
                status         = ((btf_action_fn_t)p_node->handler_fn)(tree);
                p_node->status = status;
#if BTF_DEBUG_PRINTF_ENABLED
                printf("Action %s [%d] executed, status = %s\n", p_node->name,
                       BTF_NODE_ARRAY_INDEX(tree->nodes, p_node),
                       btf_global_action_status_string[status]);
#endif
            }
            else
            {
                p_node   = &tree->nodes[p_node->child];
                p_parent = &tree->nodes[p_node->parent];
            }

            if (BTF_RUNNING_STATUS == status)
            {
                // Store the running node index
                tree->running_node_index =
                    BTF_NODE_ARRAY_INDEX(tree->nodes, p_node);
                // (((uintptr_t) p_node) - ((uintptr_t) tree->nodes))
                // / (uintptr_t) sizeof(struct btf_node);
#if BTF_DEBUG_PRINTF_ENABLED
                printf("Store RUNNING node [%u]\n",
                       (uint32_t) tree->running_node_index);
#endif
            }
        }
        else
        {
            // Check node status with policy function from parent node
            exec_res = ((btf_control_fn_t)p_parent->handler_fn)(tree, p_node, &status);
#if BTF_DEBUG_PRINTF_ENABLED
            printf("Policy %s [%d] executed, result = %s, ret. status = %s\n",
                   p_parent->name, BTF_NODE_ARRAY_INDEX(tree->nodes, p_parent),
                   btf_global_control_status_string[exec_res],
                   btf_global_action_status_string[status]);
#endif

            if (BTF_CONTINUE_EXECUTION_RESULT == exec_res)
            {
                if (p_node->sibling != BTF_NULL_NODE)
                {
                    p_node   = &tree->nodes[p_node->sibling];
                    p_parent = &tree->nodes[p_node->parent];
                    status   = BTF_UNDEF_STATUS;
                }
                else
                {
                    p_parent->status = status;
                    p_node           = &tree->nodes[p_node->parent];
                    p_parent         = &tree->nodes[p_node->parent];
                }
            }
            else if (BTF_RETURN_EXECUTION_RESULT == exec_res)
            {
                p_parent->status = status;
                p_node           = &tree->nodes[p_node->parent];
                p_parent         = &tree->nodes[p_node->parent];
            }
            else
            {
                // BTF_UNDEF_EXECUTION_RESULT ||
                // BTF_PAUSE_EXECUTION_RESULT
            }
        }
    }

    return status;
}

int32_t btf_tree_controller(struct btf_tree *tree)
{
    if (NULL == tree)
    {
        return BTF_ERROR_EINVAL;
    }

    return 0;
}
