/**
 * @file btf_tree_runner_baremetal.c
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief
 * @version 0.1
 * @date 2026-02-12
 *
 */

#include <stdio.h>

#include "btreefy/btf_tree_runner.h"

static btf_tree_st      *tree_ptr      = NULL;
static volatile uint32_t runner_events = 0;

int32_t btf_runner_init(btf_tree_st *tree)
{
    if (tree == NULL)
    {
        return -1;
    }

    tree_ptr      = tree;
    runner_events = 0;

    printf("%s executed\n", __func__);

    return 0;
}

int32_t btf_runner_notify_event(uint32_t evt)
{
    runner_events |= evt;

    return 0;
}

void btf_runner_execute(void)
{
    uint32_t events_to_wait = BTF_RUNNER_TICK_REQUEST_EVT | BTF_RUNNER_TIME_EVT
                              | BTF_RUNNER_BLACKBOARD_EVT;
    uint32_t current_events;

    current_events = runner_events;

    if ((current_events & events_to_wait) != 0)
    {
        runner_events &= ~current_events;  // Clear handled events

        printf("%s: tick tree\n", __func__);
        btf_tick_tree(tree_ptr);
    }
}
