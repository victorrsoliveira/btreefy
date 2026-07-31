/*
 * Copyright (c) 2026 Victor Oliveira
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "app_blackboard.h"
#include "btreefy/btreefy.h"
#include "door_operator_controller.h"

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000

extern struct btf_node nodes[];
extern size_t          nodes_size;

struct app_blackboard app_blackboard_ctx = {
    .button_emergency = false,
    .open_request = false,
    .close_request = false,
    .has_emergency_occurred = false,
    .door_status = BTFDT_DOOR_IS_OPEN,
    .motor_status = BTFDT_MOTOR_STOP_DOOR
};
pthread_mutex_t app_blackboard_mutex = PTHREAD_MUTEX_INITIALIZER;

// ### BT ACTIONS - START ###

enum btf_node_status stop_door_action(struct btf_tree *tree, void *data,
                                   size_t datalen)
{
    printf("Door has stopped.\n");
    door_operator_ctrl_set_action(STOP_DOOR);
    return BTF_SUCCESS_STATUS;
}

enum btf_node_status open_door_action(struct btf_tree *tree, void *data,
                                   size_t datalen)
{
    enum btfdt_door_sensor_status door_status;

    printf("Door is opening... ");
    pthread_mutex_lock(&app_blackboard_mutex);
    door_status = app_blackboard_ctx.door_status;
    pthread_mutex_unlock(&app_blackboard_mutex);

    if (door_status != BTFDT_DOOR_IS_OPEN)
    {
        printf("request ACCEPTED!\n");
        door_operator_ctrl_set_action(OPEN_DOOR);
    }
    else
    {
        printf("but already OPEN, REJECTED!\n");
    }
    return BTF_SUCCESS_STATUS;
}

enum btf_node_status close_door_action(struct btf_tree *tree, void *data,
                                    size_t datalen)
{
    printf("Door is closing... ");
    door_operator_ctrl_set_action(CLOSE_DOOR);
    return BTF_SUCCESS_STATUS;
}

enum btf_node_status emergency_action(struct btf_tree *tree, void *data,
                                   size_t datalen)
{
    bool flag_occurred = true;
    printf("Emergency occurred!\n");
    pthread_mutex_lock(&app_blackboard_mutex);
    app_blackboard_ctx.has_emergency_occurred = flag_occurred;
    pthread_mutex_unlock(&app_blackboard_mutex);
    return BTF_SUCCESS_STATUS;
}

// ### BT ACTIONS - END ###

// ### BT CONDITIONS - START ###

enum btf_node_status is_motor_on_cond(struct btf_tree *tree, void *data,
                                   size_t datalen)
{
    enum btfdt_motor_action_status motor_status;
    pthread_mutex_lock(&app_blackboard_mutex);
    motor_status = app_blackboard_ctx.motor_status;
    pthread_mutex_unlock(&app_blackboard_mutex);
    return (motor_status == BTFDT_MOTOR_STOP_DOOR) ? BTF_FAILURE_STATUS
                                                   : BTF_SUCCESS_STATUS;
}

enum btf_node_status is_emerg_btn_pressed_cond(struct btf_tree *tree, void *data,
                                            size_t datalen)
{
    enum btf_node_status ret = BTF_FAILURE_STATUS;
    bool              flag_occurred;

    pthread_mutex_lock(&app_blackboard_mutex);
    flag_occurred = app_blackboard_ctx.button_emergency;
    pthread_mutex_unlock(&app_blackboard_mutex);
    if (flag_occurred)
    {
        printf("Emergency button pressed!\n");
        flag_occurred = false;
        pthread_mutex_lock(&app_blackboard_mutex);
        app_blackboard_ctx.button_emergency = flag_occurred;
        pthread_mutex_unlock(&app_blackboard_mutex);
        ret = BTF_SUCCESS_STATUS;
    }
    return ret;
}

enum btf_node_status is_open_cond(struct btf_tree *tree, void *data, size_t datalen)
{
    enum btf_node_status             ret = BTF_FAILURE_STATUS;
    enum btfdt_door_sensor_status door_status;

    pthread_mutex_lock(&app_blackboard_mutex);
    door_status = app_blackboard_ctx.door_status;
    pthread_mutex_unlock(&app_blackboard_mutex);
    if (door_status == BTFDT_DOOR_IS_OPEN)
    {
        ret = BTF_SUCCESS_STATUS;
        printf("Door is open\n");
    }
    else if (door_status == BTFDT_DOOR_IS_CLOSED)
    {
        printf("Door is closed\n");
    }
    return ret;
}

enum btf_node_status is_opening_cond(struct btf_tree *tree, void *data, size_t datalen)
{
    enum btf_node_status              ret = BTF_FAILURE_STATUS;
    enum btfdt_door_sensor_status  door_status;
    enum btfdt_motor_action_status motor_status;

    pthread_mutex_lock(&app_blackboard_mutex);
    door_status = app_blackboard_ctx.door_status;
    pthread_mutex_unlock(&app_blackboard_mutex);
    pthread_mutex_lock(&app_blackboard_mutex);
    motor_status = app_blackboard_ctx.motor_status;
    pthread_mutex_unlock(&app_blackboard_mutex);

    if ((door_status == BTFDT_DOOR_IS_UNDEFINED)
        && (motor_status == BTFDT_MOTOR_OPEN_DOOR))
    {
        ret = BTF_SUCCESS_STATUS;
    }
    return ret;
}

enum btf_node_status is_closed_cond(struct btf_tree *tree, void *data, size_t datalen)
{
    enum btf_node_status             ret = BTF_FAILURE_STATUS;
    enum btfdt_door_sensor_status door_status;

    pthread_mutex_lock(&app_blackboard_mutex);
    door_status = app_blackboard_ctx.door_status;
    pthread_mutex_unlock(&app_blackboard_mutex);
    if (door_status == BTFDT_DOOR_IS_CLOSED)
    {
        ret = BTF_SUCCESS_STATUS;
        printf("Door is closed\n");
    }
    else if (door_status == BTFDT_DOOR_IS_OPEN)
    {
        printf("Door is open\n");
    }
    return ret;
}

enum btf_node_status is_closing_cond(struct btf_tree *tree, void *data, size_t datalen)
{
    enum btf_node_status              ret = BTF_FAILURE_STATUS;
    enum btfdt_door_sensor_status  door_status;
    enum btfdt_motor_action_status motor_status;

    pthread_mutex_lock(&app_blackboard_mutex);
    door_status = app_blackboard_ctx.door_status;
    pthread_mutex_unlock(&app_blackboard_mutex);
    pthread_mutex_lock(&app_blackboard_mutex);
    motor_status = app_blackboard_ctx.motor_status;
    pthread_mutex_unlock(&app_blackboard_mutex);

    if ((door_status == BTFDT_DOOR_IS_UNDEFINED)
        && (motor_status == BTFDT_MOTOR_CLOSE_DOOR))
    {
        ret = BTF_SUCCESS_STATUS;
    }
    return ret;
}

enum btf_node_status has_emergency_ocurred_cond(struct btf_tree *tree, void *data,
                                             size_t datalen)
{
    enum btf_node_status ret = BTF_FAILURE_STATUS;
    bool              flag_occurred;

    pthread_mutex_lock(&app_blackboard_mutex);
    flag_occurred = app_blackboard_ctx.has_emergency_occurred;
    pthread_mutex_unlock(&app_blackboard_mutex);
    if (flag_occurred)
    {
        flag_occurred = false;
        pthread_mutex_lock(&app_blackboard_mutex);
        app_blackboard_ctx.has_emergency_occurred = flag_occurred;
        pthread_mutex_unlock(&app_blackboard_mutex);
        ret = BTF_SUCCESS_STATUS;
    }
    return ret;
}

enum btf_node_status open_door_request_cond(struct btf_tree *tree, void *data,
                                         size_t datalen)
{
    enum btf_node_status ret = BTF_FAILURE_STATUS;
    bool              flag_occurred;

    pthread_mutex_lock(&app_blackboard_mutex);
    flag_occurred = app_blackboard_ctx.open_request;
    pthread_mutex_unlock(&app_blackboard_mutex);
    if (flag_occurred)
    {
        printf("Open door request!\n");
        flag_occurred = false;
        pthread_mutex_lock(&app_blackboard_mutex);
        app_blackboard_ctx.open_request = flag_occurred;
        pthread_mutex_unlock(&app_blackboard_mutex);
        ret = BTF_SUCCESS_STATUS;
    }
    return ret;
}

enum btf_node_status close_door_request_cond(struct btf_tree *tree, void *data,
                                          size_t datalen)
{
    enum btf_node_status ret = BTF_FAILURE_STATUS;
    bool              flag_occurred;

    pthread_mutex_lock(&app_blackboard_mutex);
    flag_occurred = app_blackboard_ctx.close_request;
    pthread_mutex_unlock(&app_blackboard_mutex);
    if (flag_occurred)
    {
        printf("Close door request!\n");
        flag_occurred = false;
        pthread_mutex_lock(&app_blackboard_mutex);
        app_blackboard_ctx.close_request = flag_occurred;
        pthread_mutex_unlock(&app_blackboard_mutex);
        ret = BTF_SUCCESS_STATUS;
    }
    return ret;
}

// ### BT CONDITIONS - END ###

static void *scenario_runner_thread(void *arg)
{
    bool flag_occurred = true;

    sleep(2);

    // 1. Emergency Button
    printf("\n--- SCENARIO: Emergency Button Pressed ---\n");
    pthread_mutex_lock(&app_blackboard_mutex);
    app_blackboard_ctx.button_emergency = flag_occurred;
    pthread_mutex_unlock(&app_blackboard_mutex);

    sleep(5);

    // 2. Open Request
    printf("\n--- SCENARIO: Open Request ---\n");
    pthread_mutex_lock(&app_blackboard_mutex);
    app_blackboard_ctx.open_request = flag_occurred;
    pthread_mutex_unlock(&app_blackboard_mutex);

    sleep(10);

    // 3. Close Request
    printf("\n--- SCENARIO: Close Request ---\n");
    pthread_mutex_lock(&app_blackboard_mutex);
    app_blackboard_ctx.close_request = flag_occurred;
    pthread_mutex_unlock(&app_blackboard_mutex);

    sleep(10);

    printf("\n--- SCENARIO COMPLETE ---\n");
    exit(0);

    return NULL;
}

int main(void)
{
    struct btf_tree tree;
    pthread_t   scenario_thread;

    if (door_operator_ctrl_init())
    {
        printf("Failed to init door controller\n");
        return -1;
    }

    if (btf_init(&tree, nodes, nodes_size) != 0)
    {
        printf("Failed to init tree\n");
        return -1;
    }

    // Create scenario thread to drive blackboard updates
    if (pthread_create(&scenario_thread, NULL, scenario_runner_thread, NULL) != 0)
    {
        printf("Failed to create scenario thread\n");
        return -1;
    }

    // Main loop: tick the tree manually
    while (1)
    {
        btf_tick_tree(&tree);
        usleep(100 * 1000); // 100ms
    }

    return 0;
}