/**
 * @file btreefy_always_actions.h
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2026-04-30
 * 
 */

#ifndef BTREEFY_ALWAYS_ACTIONS_H
#define BTREEFY_ALWAYS_ACTIONS_H

#include "btreefy_objs.h"

enum btf_node_status btf_always_success_action(struct btf_tree *tree, void *data, size_t datalen);
enum btf_node_status btf_always_failure_action(struct btf_tree *tree, void *data, size_t datalen);

#endif  // BTREEFY_ALWAYS_ACTIONS_H
