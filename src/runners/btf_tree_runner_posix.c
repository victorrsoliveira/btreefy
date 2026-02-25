/**
 * @file btf_tree_runner_posix.c
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-07-28
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "btreefy/btf_tree_runner.h"

static btf_tree_st *tree_ptr = NULL;

static pthread_t       runner_thread;
static pthread_mutex_t runner_mutex;
static pthread_cond_t  runner_cond;
static uint32_t        runner_events = 0;

static void *btf_runner_thread(void *arg);

int32_t btf_runner_init(btf_tree_st *tree)
{
    int err;

    if (tree == NULL)
    {
        return -1;
    }

    tree_ptr = tree;

    if (pthread_mutex_init(&runner_mutex, NULL) != 0)
    {
        return -1;
    }

    if (pthread_cond_init(&runner_cond, NULL) != 0)
    {
        return -1;
    }

    err = pthread_create(&runner_thread, NULL, btf_runner_thread, NULL);
    if (err != 0)
    {
        printf("POSIX thread creation failed: %d\n", err);
        return -1;
    }

    printf("%s executed\n", __func__);

    return 0;
}

int32_t btf_runner_notify_event(uint32_t evt)
{
    pthread_mutex_lock(&runner_mutex);
    runner_events |= evt;
    pthread_cond_signal(&runner_cond);
    pthread_mutex_unlock(&runner_mutex);

    return 0;
}

static void *btf_runner_thread(void *arg)
{
    uint32_t events_to_wait = BTF_RUNNER_TICK_REQUEST_EVT | BTF_RUNNER_TIME_EVT
                              | BTF_RUNNER_BLACKBOARD_EVT;
    uint32_t current_events;

    printf("btf_runner_thread has started\n");

    while (true)
    {
        pthread_mutex_lock(&runner_mutex);
        while ((runner_events & events_to_wait) == 0)
        {
            pthread_cond_wait(&runner_cond, &runner_mutex);
        }

        current_events = runner_events;
        runner_events &= ~current_events;  // Clear handled events
        pthread_mutex_unlock(&runner_mutex);

        printf("%s: tick tree\n", __func__);
        btf_tick_tree(tree_ptr);
    }

    return NULL;
}
