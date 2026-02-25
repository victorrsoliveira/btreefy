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
#include <string.h>
#include <unistd.h>

#include "btreefy/btf_tree_runner.h"

static struct btf_tree *tree_ptr = NULL;

struct btf_runner_data
{
    pthread_t       thread;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    uint32_t        events;
};

static void *btf_runner_thread(void *arg);

int32_t btf_runner_init(struct btf_runner *runner, btf_tree_st *tree,
                        struct btf_runner_config *config)
{
    int       err;
    pthread_t thread_id;

    // Validate input
    if ((runner == NULL) || (tree == NULL) || (config == NULL))
    {
        return -1;
    }

    runner->tree = tree;
    memcpy(&runner->config, config, sizeof(struct btf_runner_config));

    err = pthread_create(&thread_id, NULL, btf_runner_thread, runner);
    if (err != 0)
    {
        printf("POSIX thread creation failed: %d\n", err);
        return -1;
    }

    runner->is_running = false;
    runner->data = NULL;

    printf("%s executed\n", __func__);

    return 0;
}

int32_t btf_runner_start(struct btf_runner *runner)
{
    return 0;
}

int32_t btf_runner_stop(struct btf_runner *runner)
{
    return 0;
}

int32_t btf_runner_notify_event(struct btf_runner *runner, uint32_t evt)
{
    // Validate input
    if (runner == NULL)
    {
        return -1;
    }

    if (runner->data == NULL)
    {
        // Runner thread has not created runner data already
        return -2;
    }
    pthread_mutex_lock(&runner->data->mutex);
    runner->data->events |= evt;
    pthread_cond_signal(&runner->data->cond);
    pthread_mutex_unlock(&runner->data->mutex);

    return 0;
}

static void *btf_runner_thread(void *arg)
{
    struct btf_runner     *runner = (struct btf_runner *) arg;

    // Thread data
    struct btf_runner_data runner_data = {0};

    uint32_t events_to_wait = BTF_RUNNER_TICK_REQUEST_EVT | BTF_RUNNER_TIME_EVT
                              | BTF_RUNNER_BLACKBOARD_EVT;
    uint32_t current_events;

    if (pthread_mutex_init(&runner_data.mutex, NULL) != 0)
    {
        return -1;
    }

    if (pthread_cond_init(&runner_data.cond, NULL) != 0)
    {
        return -1;
    }

    runner_data.thread = pthread_self();
    runner->data = &runner_data;

    printf("btf_runner_thread has started\n");

    runner->is_running = true;

    while (true)
    {
        pthread_mutex_lock(&runner->data->mutex);
        while ((runner->data->events & events_to_wait) == 0)
        {
            pthread_cond_wait(&runner->data->cond, &runner->data->mutex);
        }
        // TODO: Add new exit event to break the loop and stop thread
        if (runner->data->events & BTF_RUNNER_EXIT_EVT)
        {
            break;
        }

        current_events = runner->data->events;
        runner->data->events &= ~current_events;  // Clear handled events
        pthread_mutex_unlock(&runner->data->mutex);

        printf("%s: tick tree\n", __func__);
        btf_tick_tree(tree_ptr);
    }

    // TODO: Validate if this is correct
    runner->is_running = false;

    // TODO: Clean runner state

    return NULL;
}
