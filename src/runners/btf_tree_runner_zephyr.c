/**
 * @file btf_tree_runner_zephyr.c
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-07-28
 *
 */

#include "btreefy/btf_tree_runner.h"

#include <zephyr/kernel.h>

#define BTF_RUNNER_THREAD_STACK_SIZE 512
#define BTF_RUNNER_THREAD_PRIORITY   5

K_EVENT_DEFINE(runner_event);

static struct btf_tree *tree_ptr = NULL;

static void btf_runner_thread(void *, void *, void *);

K_THREAD_DEFINE(btf_runner_thread_tid, BTF_RUNNER_THREAD_STACK_SIZE,
                btf_runner_thread, NULL, NULL, NULL, BTF_RUNNER_THREAD_PRIORITY,
                0, 0);

int32_t btf_runner_init(struct btf_tree *tree)
{
    if (tree == NULL)
    {
        return -1;
    }

    tree_ptr = tree;

    printk("%s executed\n", __func__);

    return 0;
}

int32_t btf_runner_notify_event(uint32_t evt)
{
    k_event_post(&runner_event, evt);

    return 0;
}

static void btf_runner_thread(void *param1, void *param2, void *param3)
{
    uint32_t events;
    uint32_t events_to_wait = BTF_RUNNER_TICK_REQUEST_EVT | BTF_RUNNER_TIME_EVT
                              | BTF_RUNNER_BLACKBOARD_EVT;

    printk("%s has started\n", __func__);

    while (true)
    {
        events = k_event_wait(&runner_event, events_to_wait, false, K_FOREVER);

        k_event_set_masked(&runner_event, ~events, events);

        printk("%s: tick tree\n", __func__);
        btf_tick_tree(tree_ptr);
    }
}
