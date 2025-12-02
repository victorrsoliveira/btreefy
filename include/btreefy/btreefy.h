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

#include "btreefy/btreefy_objs.h"
#include "btreefy/btreefy_policies.h"

#define BTF_DEBUG_PRINTF_ENABLED 1

#define BTF_ERROR_OK     0U
#define BTF_ERROR_EINVAL -1
#define BTF_ERROR_EXEC   -2

#define BTF_NODE_ARRAY_INDEX(p_nodes, p_node)                     \
    (uint32_t)((((uintptr_t) (p_node)) - ((uintptr_t) (p_nodes))) \
               / (uintptr_t) sizeof(struct btf_node))

int32_t btf_init(btf_tree_st *tree, struct btf_node *nodes, uint32_t tree_size);

int32_t btf_tick_tree(btf_tree_st *tree);

int32_t btf_tree_controller(btf_tree_st *tree);

int32_t btf_blackboard_update_data(int16_t index, void *data, size_t size);

int32_t btf_blackboard_retrieve_data(int16_t index, void *data, size_t size);

#endif  // BTREEFY_H
