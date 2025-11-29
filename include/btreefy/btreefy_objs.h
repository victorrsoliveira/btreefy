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

#define BTF_NULL_NODE 0xFFFFFFFF

/**
 * @brief Node status enumeration
 *
 */
typedef enum
{
    BTF_UNDEF_STATUS,
    BTF_SUCCESS_STATUS,
    BTF_FAILURE_STATUS,
    BTF_RUNNING_STATUS
} btf_node_status_t;

typedef enum
{
    BTF_UNDEF_EXECUTION_RESULT,
    BTF_CONTINUE_EXECUTION_RESULT,
    BTF_RETURN_EXECUTION_RESULT,
    BTF_PAUSE_EXECUTION_RESULT
} btf_node_execution_result_t;

/**
 * @brief
 *
 */
struct btf_node;

typedef struct
{
    struct btf_node *nodes;
    uint32_t         size;
    uintptr_t        running_node_index;
} btf_tree_st;

struct btf_node
{
    btf_node_status_t status;
    uint32_t          parent;
    uint32_t          child;
    uint32_t          sibling;
    btf_node_status_t (*action)(btf_tree_st *tree, void *data, size_t datalen);
    btf_node_execution_result_t (*control)(btf_tree_st       *tree,
                                           struct btf_node   *child_node,
                                           btf_node_status_t *status,
                                           void *data, size_t datalen);
    char *name;
};

// WARNING: Read/Write to checked and n_unchecked_entries MUST be in
// CRITICAL_SECTION

#define BTF_BLKBRD_DATA()

struct btf_blackboard_data
{
    void  *data;
    size_t size;
    # ifdef 0
    bool (*validator)(void *data, size_t size);
    #endif
    bool checked;
};

struct btf_blackboard
{
    struct btf_blackboard_data *table;
    size_t                      n_entries;
    size_t                      n_unchecked_entries;
};

extern char *btf_global_action_status_string[];

/**
 * @brief Function prototype definition of an action executed by an Action node
 * in the tree
 *
 */
typedef btf_node_status_t (*btf_action_fn_t)(btf_tree_st *tree, void *data,
                                             size_t datalen);

/**
 * @brief Function prototype definition for a policy executed by a Control node
 * in the tree
 *
 */
typedef btf_node_execution_result_t (*btf_control_fn_t)(
    btf_tree_st *tree, struct btf_node *child_node, btf_node_status_t *status,
    void *data, size_t datalen);


#endif  // BTREEFY_OBJS_H
