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

#ifdef CONFIG_BTREEFY_DEBUG_PRINTF
#define BTF_DEBUG_PRINTF_ENABLED 1
#else
#define BTF_DEBUG_PRINTF_ENABLED 0
#endif

#if defined(CONFIG_BTREEFY_NODE_INDEX_16BIT)
typedef uint16_t btf_node_index_t;
#define BTF_NULL_NODE ((btf_node_index_t)0xFFFF)
#elif defined(CONFIG_BTREEFY_NODE_INDEX_32BIT)
typedef uint32_t btf_node_index_t;
#define BTF_NULL_NODE ((btf_node_index_t)0xFFFFFFFF)
#else
typedef uint8_t btf_node_index_t;
#define BTF_NULL_NODE ((btf_node_index_t)0xFF)
#endif

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
    size_t           size;
    void *           data;
    size_t           datalen;
    btf_node_index_t running_node_index;
};

static inline void btf_tree_copy_data(struct btf_tree * tree, void * data, size_t datalen)
{
    // NOTE: This function should only be called when @p data is guaranteed to be non-null
    if (datalen >= tree->datalen)
    {
        memcpy(data, tree->data, tree->datalen);
    }
}

typedef enum btf_node_status (*btf_action_fn_t)(struct btf_tree *tree);
typedef enum btf_node_execution_result (*btf_control_fn_t)(
    struct btf_tree *tree, struct btf_node *child_node,
    enum btf_node_status *status);

struct btf_node
{
    enum btf_node_status status;
    btf_node_index_t  parent;
    btf_node_index_t  child;
    btf_node_index_t  sibling;
    void             *handler_fn;
#if BTF_DEBUG_PRINTF_ENABLED
    char *name;
#endif
};

extern char *btf_global_action_status_string[];

#endif  // BTREEFY_OBJS_H
