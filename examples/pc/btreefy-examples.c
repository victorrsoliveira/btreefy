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
#include "btreefy/btf_blackboard.h"
#include "btreefy/btf_tree_runner.h"
#include "btreefy/btreefy.h"
#include "door_operator_controller.h"

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000

extern struct btf_node nodes[];
extern size_t          nodes_size;

BTF_BLACKBOARD_DEFINE(
    app_blackboard, struct app_blackboard,
    BTF_BLACKBOARD_INIT_VAL(.button_emergency = false, .open_request = false,
                            .close_request          = false,
                            .has_emergency_occurred = false,
                            .door_status            = BTFDT_DOOR_IS_OPEN,
                            .motor_status           = BTFDT_MOTOR_STOP_DOOR));

// ### BT ACTIONS - START ###

btf_node_status_t stop_door_action(btf_tree_st *tree, void *data,
                                   size_t datalen)
{
    printf("Door has stopped.\n");
    door_operator_ctrl_set_action(STOP_DOOR);
    return BTF_SUCCESS_STATUS;
}

btf_node_status_t open_door_action(btf_tree_st *tree, void *data,
                                   size_t datalen)
{
    enum btfdt_door_sensor_status door_status;

    printf("Door is opening... ");
    BTF_BLACKBOARD_RETRIEVE_DATA(app_blackboard, door_status, door_status);

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

btf_node_status_t close_door_action(btf_tree_st *tree, void *data,
                                    size_t datalen)
{
    printf("Door is closing... ");
    door_operator_ctrl_set_action(CLOSE_DOOR);
    return BTF_SUCCESS_STATUS;
}

btf_node_status_t emergency_action(btf_tree_st *tree, void *data,
                                   size_t datalen)
{
    bool flag_occurred = true;
    printf("Emergency occurred!\n");
    BTF_BLACKBOARD_UPDATE_DATA(app_blackboard, has_emergency_occurred,
                               flag_occurred);
    return BTF_SUCCESS_STATUS;
}

// ### BT ACTIONS - END ###

// ### BT CONDITIONS - START ###

btf_node_status_t is_motor_on_cond(btf_tree_st *tree, void *data,
                                   size_t datalen)
{
    enum btfdt_motor_action_status motor_status;
    BTF_BLACKBOARD_RETRIEVE_DATA(app_blackboard, motor_status, motor_status);
    return (motor_status == BTFDT_MOTOR_STOP_DOOR) ? BTF_FAILURE_STATUS
                                                   : BTF_SUCCESS_STATUS;
}

btf_node_status_t is_emerg_btn_pressed_cond(btf_tree_st *tree, void *data,
                                            size_t datalen)
{
    btf_node_status_t ret = BTF_FAILURE_STATUS;
    bool              flag_occurred;

    BTF_BLACKBOARD_RETRIEVE_DATA(app_blackboard, button_emergency,
                                 flag_occurred);
    if (flag_occurred)
    {
        printf("Emergency button pressed!\n");
        flag_occurred = false;
        BTF_BLACKBOARD_UPDATE_DATA(app_blackboard, button_emergency,
                                   flag_occurred);
        ret = BTF_SUCCESS_STATUS;
    }
    return ret;
}

btf_node_status_t is_open_cond(btf_tree_st *tree, void *data, size_t datalen)
{
    btf_node_status_t             ret = BTF_FAILURE_STATUS;
    enum btfdt_door_sensor_status door_status;

    BTF_BLACKBOARD_RETRIEVE_DATA(app_blackboard, door_status, door_status);
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

btf_node_status_t is_opening_cond(btf_tree_st *tree, void *data, size_t datalen)
{
    btf_node_status_t              ret = BTF_FAILURE_STATUS;
    enum btfdt_door_sensor_status  door_status;
    enum btfdt_motor_action_status motor_status;

    BTF_BLACKBOARD_RETRIEVE_DATA(app_blackboard, door_status, door_status);
    BTF_BLACKBOARD_RETRIEVE_DATA(app_blackboard, motor_status, motor_status);

    if ((door_status == BTFDT_DOOR_IS_UNDEFINED)
        && (motor_status == BTFDT_MOTOR_OPEN_DOOR))
    {
        ret = BTF_SUCCESS_STATUS;
    }
    return ret;
}

btf_node_status_t is_closed_cond(btf_tree_st *tree, void *data, size_t datalen)
{
    btf_node_status_t             ret = BTF_FAILURE_STATUS;
    enum btfdt_door_sensor_status door_status;

    BTF_BLACKBOARD_RETRIEVE_DATA(app_blackboard, door_status, door_status);
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

btf_node_status_t is_closing_cond(btf_tree_st *tree, void *data, size_t datalen)
{
    btf_node_status_t              ret = BTF_FAILURE_STATUS;
    enum btfdt_door_sensor_status  door_status;
    enum btfdt_motor_action_status motor_status;

    BTF_BLACKBOARD_RETRIEVE_DATA(app_blackboard, door_status, door_status);
    BTF_BLACKBOARD_RETRIEVE_DATA(app_blackboard, motor_status, motor_status);

    if ((door_status == BTFDT_DOOR_IS_UNDEFINED)
        && (motor_status == BTFDT_MOTOR_CLOSE_DOOR))
    {
        ret = BTF_SUCCESS_STATUS;
    }
    return ret;
}

btf_node_status_t has_emergency_ocurred_cond(btf_tree_st *tree, void *data,
                                             size_t datalen)
{
    btf_node_status_t ret = BTF_FAILURE_STATUS;
    bool              flag_occurred;

    BTF_BLACKBOARD_RETRIEVE_DATA(app_blackboard, has_emergency_occurred,
                                 flag_occurred);
    if (flag_occurred)
    {
        flag_occurred = false;
        BTF_BLACKBOARD_UPDATE_DATA(app_blackboard, has_emergency_occurred,
                                   flag_occurred);
        ret = BTF_SUCCESS_STATUS;
    }
    return ret;
}

btf_node_status_t open_door_request_cond(btf_tree_st *tree, void *data,
                                         size_t datalen)
{
    btf_node_status_t ret = BTF_FAILURE_STATUS;
    bool              flag_occurred;

    BTF_BLACKBOARD_RETRIEVE_DATA(app_blackboard, open_request, flag_occurred);
    if (flag_occurred)
    {
        printf("Open door request!\n");
        flag_occurred = false;
        BTF_BLACKBOARD_UPDATE_DATA(app_blackboard, open_request, flag_occurred);
        ret = BTF_SUCCESS_STATUS;
    }
    return ret;
}

btf_node_status_t close_door_request_cond(btf_tree_st *tree, void *data,
                                          size_t datalen)
{
    btf_node_status_t ret = BTF_FAILURE_STATUS;
    bool              flag_occurred;

    BTF_BLACKBOARD_RETRIEVE_DATA(app_blackboard, close_request, flag_occurred);
    if (flag_occurred)
    {
        printf("Close door request!\n");
        flag_occurred = false;
        BTF_BLACKBOARD_UPDATE_DATA(app_blackboard, close_request,
                                   flag_occurred);
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
    BTF_BLACKBOARD_UPDATE_DATA(app_blackboard, button_emergency, flag_occurred);

    sleep(5);

    // 2. Open Request
    printf("\n--- SCENARIO: Open Request ---\n");
    BTF_BLACKBOARD_UPDATE_DATA(app_blackboard, open_request, flag_occurred);

    sleep(10);

    // 3. Close Request
    printf("\n--- SCENARIO: Close Request ---\n");
    BTF_BLACKBOARD_UPDATE_DATA(app_blackboard, close_request, flag_occurred);

    sleep(10);

    printf("\n--- SCENARIO COMPLETE ---\n");
    exit(0);

    return NULL;
}

int main(void)
{
    btf_tree_st tree;
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

    if (btf_runner_init(&tree))
    {
        printf("Failed to init runner\n");
        return -1;
    }

    // Create scenario thread to drive blackboard updates
    if (pthread_create(&scenario_thread, NULL, scenario_runner_thread, NULL) != 0)
    {
        printf("Failed to create scenario thread\n");
        return -1;
    }

    // Main loop: runner thread handles all ticking via blackboard events
    while (1)
    {
        usleep(SLEEP_TIME_MS * 1000);
    }

    return 0;
}