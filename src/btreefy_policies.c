/**
 * @file btreefy_policies.c
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-03-20
 *
 */

#include "btreefy_policies.h"

#include <stdio.h>

btf_node_execution_result_t btf_sequence_policy_fn(btf_tree_st     *tree,
                                                   struct btf_node *child_node,
                                                   btf_node_status_t *status,
                                                   void *data, size_t datalen)
{
    if (status == NULL)
    {
        printf("Error on sequence policy execution: status is NULL\n");
        return BTF_RETURN_EXECUTION_RESULT;
    }

    // There is a node in RUNNING state
    if ((tree->running_node_index != BTF_NULL_NODE)
        && (*status == BTF_FAILURE_STATUS)
        && (tree->nodes[child_node->parent].status == BTF_RUNNING_STATUS)
        && (tree->nodes[tree->running_node_index].status == BTF_RUNNING_STATUS))
    {
        printf("Must abort RUNNING action [%lu]\n", tree->running_node_index);
        tree->running_node_index = BTF_NULL_NODE;
    }

    if (BTF_SUCCESS_STATUS == *status)
    {
        return BTF_CONTINUE_EXECUTION_RESULT;
    }
    else if (BTF_FAILURE_STATUS == *status)
    {
        return BTF_RETURN_EXECUTION_RESULT;
    }
    else if (BTF_RUNNING_STATUS == *status)
    {
        return BTF_RETURN_EXECUTION_RESULT;
    }
    else
    {
        printf("SEQUENCE: WRONG STATUS\n");
        // Default
    }

    return BTF_RETURN_EXECUTION_RESULT;
}

btf_node_execution_result_t btf_fallback_policy_fn(btf_tree_st     *tree,
                                                   struct btf_node *child_node,
                                                   btf_node_status_t *status,
                                                   void *data, size_t datalen)
{
    if (status == NULL)
    {
        printf("Error on fallback policy execution: status is NULL\n");
        return BTF_RETURN_EXECUTION_RESULT;
    }

    // There is a node in RUNNING state
    if ((tree->running_node_index != BTF_NULL_NODE)
        && (*status == BTF_SUCCESS_STATUS)
        && (tree->nodes[child_node->parent].status == BTF_RUNNING_STATUS)
        && (tree->nodes[tree->running_node_index].status == BTF_RUNNING_STATUS))
    {
        printf("Must abort RUNNING action [%lu]\n", tree->running_node_index);
        tree->running_node_index = BTF_NULL_NODE;
    }

    if (BTF_SUCCESS_STATUS == *status)
    {
        return BTF_RETURN_EXECUTION_RESULT;
    }
    else if (BTF_FAILURE_STATUS == *status)
    {
        return BTF_CONTINUE_EXECUTION_RESULT;
    }
    else if (BTF_RUNNING_STATUS == *status)
    {
        return BTF_RETURN_EXECUTION_RESULT;
    }
    else
    {
        printf("FALLBACK: WRONG STATUS\n");
        // Default
    }

    return BTF_RETURN_EXECUTION_RESULT;
}

btf_node_execution_result_t btf_success_policy_fn(btf_tree_st       *tree,
                                                  struct btf_node   *child_node,
                                                  btf_node_status_t *status,
                                                  void *data, size_t datalen)
{
    if (status == NULL)
    {
        printf("Error on success policy execution: status is NULL\n");
        return BTF_RETURN_EXECUTION_RESULT;
    }

    *status = BTF_SUCCESS_STATUS;

    return BTF_RETURN_EXECUTION_RESULT;
}

btf_node_execution_result_t btf_fail_policy_fn(btf_tree_st       *tree,
                                               struct btf_node   *child_node,
                                               btf_node_status_t *status,
                                               void *data, size_t datalen)
{
    if (status == NULL)
    {
        printf("Error on success policy execution: status is NULL\n");
        return BTF_RETURN_EXECUTION_RESULT;
    }

    *status = BTF_FAILURE_STATUS;

    return BTF_RETURN_EXECUTION_RESULT;
}