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
#define BTF_RUNNER_EXIT_EVT         8U

struct btf_runner_config
{
    bool     tick_on_update;
    uint32_t tick_interval_ms;
};

struct btf_runner_data;

struct btf_runner
{
    btf_tree_st             *tree;
    struct btf_runner_config config;
    bool                     is_running;
    struct btf_runner_data  *data;
};

int32_t btf_runner_init(struct btf_runner *runner, btf_tree_st *tree,
                        struct btf_blackboard    *blackboard,
                        struct btf_runner_config *config);

int32_t btf_runner_start(struct btf_runner *runner);

int32_t btf_runner_stop(struct btf_runner *runner);

int32_t btf_runner_notify_event(struct btf_runner *runner, uint32_t evt);

#endif  // BTF_TREE_RUNNER_H
