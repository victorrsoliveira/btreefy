/**
 * @file btreefy_policies.c
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-03-20
 *
 */

#include "btreefy/btreefy_policies.h"

#include <stdio.h>

enum btf_node_execution_result btf_sequence_policy_fn(
    struct btf_tree *tree, struct btf_node *child_node,
    enum btf_node_status *status)
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
        printf("Must abort RUNNING action [%lu]\n", (unsigned long)tree->running_node_index);
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

enum btf_node_execution_result btf_fallback_policy_fn(
    struct btf_tree *tree, struct btf_node *child_node,
    enum btf_node_status *status)
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
        printf("Must abort RUNNING action [%lu]\n", (unsigned long)tree->running_node_index);
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

enum btf_node_execution_result btf_success_policy_fn(
    struct btf_tree *tree, struct btf_node *child_node,
    enum btf_node_status *status)
{
    if (status == NULL)
    {
        printf("Error on success policy execution: status is NULL\n");
        return BTF_RETURN_EXECUTION_RESULT;
    }

    *status = BTF_SUCCESS_STATUS;

    return BTF_RETURN_EXECUTION_RESULT;
}

enum btf_node_execution_result btf_fail_policy_fn(struct btf_tree *tree,
                                                  struct btf_node *child_node,
                                                  enum btf_node_status *status)
{
    if (status == NULL)
    {
        printf("Error on success policy execution: status is NULL\n");
        return BTF_RETURN_EXECUTION_RESULT;
    }

    *status = BTF_FAILURE_STATUS;

    return BTF_RETURN_EXECUTION_RESULT;
}
