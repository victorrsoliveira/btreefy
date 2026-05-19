/**
 * @file btreefy_objs.h
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-03-20
 *
 */

#ifndef BTREEFY_OBJS_H
#define BTREEFY_OBJS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define BTF_NULL_NODE 0xFFFFFFFF

/**
 * @brief Node status enumeration
 *
 */
enum btf_node_status
{
    BTF_UNDEF_STATUS,
    BTF_SUCCESS_STATUS,
    BTF_FAILURE_STATUS,
    BTF_RUNNING_STATUS
};

enum btf_node_execution_result
{
    BTF_UNDEF_EXECUTION_RESULT,
    BTF_CONTINUE_EXECUTION_RESULT,
    BTF_RETURN_EXECUTION_RESULT,
    BTF_PAUSE_EXECUTION_RESULT
};

/**
 * @brief
 *
 */
struct btf_node;

struct btf_tree
{
    struct btf_node *nodes;
    uint32_t         size;
    void *           data;
    size_t           datalen;
    uintptr_t        running_node_index;
};

static inline void btf_tree_copy_data(struct btf_tree * tree, void * data, size_t datalen)
{
    // NOTE: This function should only be called when @p data is guaranteed to be non-null
    if (datalen >= tree->datalen)
    {
        memcpy(data, tree->data, tree->datalen);
    }
}

struct btf_node
{
    enum btf_node_status status;
    uint32_t          parent;
    uint32_t          child;
    uint32_t          sibling;
    enum btf_node_status (*action)(struct btf_tree *tree);
    enum btf_node_execution_result (*control)(struct btf_tree       *tree,
                                           struct btf_node   *child_node,
                                           enum btf_node_status *status);
    char *name;
};

extern char *btf_global_action_status_string[];

#endif  // BTREEFY_OBJS_H
