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

    printf("Sequence executed!\n");

    if (BTF_SUCCESS_STATUS == *status)
    {
        return BTF_CONTINUE_EXECUTION_RESULT;
    }
    else if (BTF_FAILURE_STATUS == *status)
    {
        return BTF_RETURN_EXECUTION_RESULT;
    }
    else
    {
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

    printf("Fallback executed!\n");

    if (BTF_SUCCESS_STATUS == *status)
    {
        return BTF_RETURN_EXECUTION_RESULT;
    }
    else if (BTF_FAILURE_STATUS == *status)
    {
        return BTF_CONTINUE_EXECUTION_RESULT;
    }
    else
    {
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

    printf("Success executed!\n");

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

    printf("Fail executed!\n");

    *status = BTF_FAILURE_STATUS;

    return BTF_RETURN_EXECUTION_RESULT;
}