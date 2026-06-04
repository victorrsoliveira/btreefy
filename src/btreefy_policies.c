/**
 * @file btreefy_policies.c
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-03-20
 *
 */

#include "btreefy/btreefy_policies.h"

#include <stdint.h>
#include <stdio.h>

#include "btreefy/btreefy_objs.h"

static uintptr_t abort_running_node(struct btf_tree *tree,
                                    uint32_t curr_node);

enum btf_node_execution_result btf_sequence_policy_fn(
    struct btf_tree *tree, struct btf_node *child_node,
    enum btf_node_status *status)
{
    uint32_t curr_node = child_node->parent;

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
        printf("Calling abort on node %s at index %d\n",
               tree->nodes[child_node->parent].name, child_node->parent);
        abort_running_node(tree, curr_node);
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
    uint32_t curr_node = child_node->parent;

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
        printf("Calling abort on node %s at index %d\n",
               tree->nodes[child_node->parent].name, child_node->parent);
        abort_running_node(tree, curr_node);
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

static uintptr_t abort_running_node(struct btf_tree *tree,
                                    uint32_t curr_node)
{
    uintptr_t curr_running_node        = tree->running_node_index;
    uintptr_t node_with_running_status = curr_running_node;

    enum btf_node_status status = BTF_UNDEF_STATUS;

    printf("Must abort RUNNING action [%lu]\n", curr_running_node);

    // ? QUESTION: Should user indicate whether the abort signal was handled?  
    // ? If so, there should be a return value for that
    status = tree->nodes[curr_running_node].action(tree, BTF_ABORT_SIGNAL);
    tree->running_node_index = BTF_NULL_NODE;

    // Clear all nodes' status from the running (aborted) node 
    // until current node being evaluated
    while ((node_with_running_status != BTF_NULL_NODE) && (node_with_running_status != curr_node) && (tree->nodes[node_with_running_status].status == BTF_RUNNING_STATUS))
    {
        printf("Cleaning RUNNING status on node at index %lu\n",
               node_with_running_status);
        tree->nodes[node_with_running_status].status = BTF_UNDEF_STATUS;
        node_with_running_status = tree->nodes[node_with_running_status].parent;
    }

    return curr_running_node;
}