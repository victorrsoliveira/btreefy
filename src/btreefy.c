/**
 * @file btreefy.c
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-03-19
 *
 */

#include "btreefy.h"

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

int32_t btf_init(btf_tree_st *tree, struct btf_node *nodes, uint32_t tree_size)

{
    if ((tree == NULL) || (nodes == NULL) || (tree_size == 0))
    {
        return BTF_ERROR_EINVAL;
    }

    tree->nodes = nodes;
    tree->size  = tree_size;

    printf("BT initialized!\n");

    return 0;
}

int32_t btf_tick_tree(btf_tree_st *tree)
{
    uint32_t                    node_index = 0;
    struct btf_node            *p_node     = NULL;
    struct btf_node            *p_parent   = NULL;
    btf_node_status_t           status     = BTF_UNDEF_STATUS;
    btf_node_execution_result_t exec_res   = BTF_UNDEF_EXECUTION_RESULT;

    if (tree == NULL)
    {
        return BTF_ERROR_EINVAL;
    }

    // Start from root node
    p_node   = &tree->nodes[0];
    p_parent = &tree->nodes[p_node->parent];

    // Loop until status is defined pointer to node is root again
    while (!((status != BTF_UNDEF_STATUS) && (p_node->parent == BTF_NULL_NODE)))
    {
        // Look for leaf node using DFS approach
        if (BTF_UNDEF_STATUS == status)
        {
            // Leaf node has been reached
            if (p_node->child == BTF_NULL_NODE)
            {
                status         = p_node->action(tree, NULL, 0);
                p_node->status = status;
                printf("Action %s [%d] executed, status = %s\n", p_node->name,
                       BTF_NODE_ARRAY_INDEX(tree->nodes, p_node),
                       btf_global_action_status_string[status]);
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
                printf("Store RUNNING node [%u]\n",
                       (uint32_t) tree->running_node_index);
            }
        }
        else
        {
            // Check node status with policy function from parent node
            exec_res = p_parent->control(tree, p_node, &status, NULL, 0);
            printf("Policy %s [%d] executed, result = %s, ret. status = %s\n",
                   p_parent->name, BTF_NODE_ARRAY_INDEX(tree->nodes, p_parent),
                   btf_global_control_status_string[exec_res],
                   btf_global_action_status_string[status]);

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

int32_t btf_tree_controller(btf_tree_st *tree)
{
    if (NULL == tree)
    {
        return BTF_ERROR_EINVAL;
    }

    return 0;
}