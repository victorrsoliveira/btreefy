/**
 * @file btreefy_policies.h
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-03-19
 *
 */

#ifndef BTREEFY_POLICIES_H
#define BTREEFY_POLICIES_H

#include "btreefy/btreefy_objs.h"

enum btf_node_execution_result btf_sequence_policy_fn(
    struct btf_tree *tree, struct btf_node *child_node,
    enum btf_node_status *status);

enum btf_node_execution_result btf_fallback_policy_fn(
    struct btf_tree *tree, struct btf_node *child_node,
    enum btf_node_status *status);


enum btf_node_execution_result btf_success_policy_fn(
    struct btf_tree *tree, struct btf_node *child_node,
    enum btf_node_status *status);


enum btf_node_execution_result btf_fail_policy_fn(struct btf_tree *tree,
                                                  struct btf_node *child_node,
                                                  enum btf_node_status *status);

#endif  // BTREEFY_POLICIES_H
