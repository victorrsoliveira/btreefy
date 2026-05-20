/**
 * @file btf_tree_runner.h
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-07-28
 *
 */

#ifndef BTF_TREE_RUNNER_H
#define BTF_TREE_RUNNER_H

#include "btreefy/btreefy.h"

#define BTF_RUNNER_TICK_REQUEST_EVT 1U
#define BTF_RUNNER_TIME_EVT         2U
#define BTF_RUNNER_BLACKBOARD_EVT   4U

int32_t btf_runner_init(struct btf_tree *tree);

int32_t btf_runner_notify_event(uint32_t evt);

#endif  // BTF_TREE_RUNNER_H
