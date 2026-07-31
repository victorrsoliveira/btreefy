/**
 * @file btreefy.h
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-03-19
 *
 */

#ifndef BTREEFY_H
#define BTREEFY_H

#include <stddef.h>
#include <stdint.h>

#include "btreefy/btreefy_objs.h"
#include "btreefy/btreefy_policies.h"

#define BTF_DEBUG_PRINTF_ENABLED 0

#define BTF_ERROR_OK     0U
#define BTF_ERROR_EINVAL -1
#define BTF_ERROR_EXEC   -2

#define BTF_NODE_ARRAY_INDEX(p_nodes, p_node)                     \
    (uint32_t)((((uintptr_t) (p_node)) - ((uintptr_t) (p_nodes))) \
               / (uintptr_t) sizeof(struct btf_node))

int32_t btf_init(struct btf_tree *tree, struct btf_node *nodes, size_t tree_size);

int32_t btf_set_data(struct btf_tree *tree, void * data, size_t datalen);

int32_t btf_tick_tree(struct btf_tree *tree);

int32_t btf_tree_controller(struct btf_tree *tree);

#endif  // BTREEFY_H
