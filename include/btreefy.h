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

#include "btreefy_objs.h"
#include "btreefy_policies.h"

#define BTF_ERROR_OK     0U
#define BTF_ERROR_EINVAL -1
#define BTF_ERROR_EXEC   -2

int32_t btf_init(btf_tree_st *tree, struct btf_node *nodes, uint32_t tree_size);

int32_t btf_tick_tree(btf_tree_st *tree);

#endif  // BTREEFY_H
