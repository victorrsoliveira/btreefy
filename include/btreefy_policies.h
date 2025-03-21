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

#include "btreefy_objs.h"

btf_node_execution_result_t btf_sequence_policy_fn(btf_tree_st     *tree,
                                                   struct btf_node *child_node,
                                                   btf_node_status_t *status,
                                                   void *data, size_t datalen);

btf_node_execution_result_t btf_fallback_policy_fn(btf_tree_st     *tree,
                                                   struct btf_node *child_node,
                                                   btf_node_status_t *status,
                                                   void *data, size_t datalen);


btf_node_execution_result_t btf_success_policy_fn(btf_tree_st       *tree,
                                                  struct btf_node   *child_node,
                                                  btf_node_status_t *status,
                                                  void *data, size_t datalen);


btf_node_execution_result_t btf_fail_policy_fn(btf_tree_st       *tree,
                                               struct btf_node   *child_node,
                                               btf_node_status_t *status,
                                               void *data, size_t datalen);

#endif  // BTREEFY_POLICIES_H
