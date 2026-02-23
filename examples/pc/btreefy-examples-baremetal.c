/*
 * Copyright (c) 2026 Victor Oliveira
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "app_blackboard.h"
#include "btreefy/btf_blackboard.h"
#include "btreefy/btf_tree_runner.h"
#include "btreefy/btreefy.h"
#include "door_operator_controller.h"

/* 100 msec polling period */
#define SLEEP_TIME_MS 100

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
        printf(" request ACCEPTED!\n");
        door_operator_ctrl_set_action(OPEN_DOOR);
    }
    else
    {
        printf(" but is alreaedy OPEN, then REJECTED!\n");
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

btf_node_status_t is_opening_cond(btf_tree_st *tree, void *data,
                                  size_t datalen)
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

btf_node_status_t is_closing_cond(btf_tree_st *tree, void *data,
                                  size_t datalen)
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

/* Bare metal scenario: drives blackboard updates from the main loop
 * using elapsed time, mimicking what scenario_runner_thread does in
 * the POSIX example via sleep(). */
static void scenario_update(void)
{
    static time_t start_time  = 0;
    static bool   stage1_done = false;
    static bool   stage2_done = false;
    static bool   stage3_done = false;

    bool   flag_occurred = true;
    time_t now;
    double elapsed;

    if (start_time == 0)
    {
        start_time = time(NULL);
    }

    now     = time(NULL);
    elapsed = difftime(now, start_time);

    // 1. Emergency Button (after 2s)
    if (elapsed >= 2 && !stage1_done)
    {
        printf("\n--- SCENARIO: Emergency Button Pressed ---\n");
        BTF_BLACKBOARD_UPDATE_DATA(app_blackboard, button_emergency,
                                   flag_occurred);
        stage1_done = true;
    }

    // 2. Open Request (after 7s)
    if (elapsed >= 7 && !stage2_done)
    {
        printf("\n--- SCENARIO: Open Request ---\n");
        BTF_BLACKBOARD_UPDATE_DATA(app_blackboard, open_request, flag_occurred);
        stage2_done = true;
    }

    // 3. Close Request (after 17s)
    if (elapsed >= 17 && !stage3_done)
    {
        printf("\n--- SCENARIO: Close Request ---\n");
        BTF_BLACKBOARD_UPDATE_DATA(app_blackboard, close_request,
                                   flag_occurred);
        stage3_done = true;
    }

    // End (after 27s)
    if (elapsed >= 27)
    {
        printf("\n--- SCENARIO COMPLETE ---\n");
        exit(0);
    }
}

int main(void)
{
    btf_tree_st tree;

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

    // Main loop: poll btf_runner_execute for events, drive scenario inline
    while (1)
    {
        btf_runner_execute();

        scenario_update();

        usleep(SLEEP_TIME_MS * 1000);
    }

    return 0;
}
