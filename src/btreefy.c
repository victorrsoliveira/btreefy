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
                status = p_node->action(tree, NULL, 0);
            }
            else
            {
                p_node   = &tree->nodes[p_node->child];
                p_parent = &tree->nodes[p_node->parent];
            }
        }
        else
        {
            // Check node status with policy function from parent node
            exec_res = p_parent->control(tree, p_node, &status, NULL, 0);

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
                    p_node   = &tree->nodes[p_node->parent];
                    p_parent = &tree->nodes[p_node->parent];
                }
            }
            else if (BTF_RETURN_EXECUTION_RESULT == exec_res)
            {
                p_node   = &tree->nodes[p_node->parent];
                p_parent = &tree->nodes[p_node->parent];
            }
            else
            {
                // BTF_UNDEF_EXECUTION_RESULT ||
                // BTF_PAUSE_EXECUTION_RESULT
            }
        }
    }

    return 0;
}
