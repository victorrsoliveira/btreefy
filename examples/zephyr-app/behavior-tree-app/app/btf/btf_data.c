/**
 * @file btf_data.c
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-07-26
 *
 */

#include <stdbool.h>

#include "btreefy/btreefy_objs.h"

enum btfdt_app_blackboard_keys
{
    BTFDT_KEY_BUTTON_EMERGENCY,
    BTFDT_KEY_HAS_EMERGENCY_OCCURRED,
    BTFDT_KEY_DOOR_STATUS,
    BTFDT_KEY_MOTOR_STATUS,
    BTFDT_BLACKBOARD_N_ENTRIES,
};

// Definition of BTreeFy model variables

static bool btfdt_button_emergency = false;

static bool btfdt_has_emergency_occurred = false;

enum btfdt_door_sensor_status
{
    BTFDT_DOOR_IS_UNDEFINED,
    BTFDT_DOOR_IS_OPEN,
    BTFDT_DOOR_IS_CLOSED
};

static enum btfdt_door_sensor_status btfdt_door_status =
    BTFDT_DOOR_IS_UNDEFINED;

enum btfdt_motor_action_status
{
    BTFDT_MOTOR_OPEN_DOOR  = -1,
    BTFDT_MOTOR_STOP_DOOR  = 0,
    BTFDT_MOTOR_CLOSE_DOOR = 1
};

static enum btfdt_motor_action_status btfdt_motor_status =
    BTFDT_MOTOR_STOP_DOOR;

// BTreeFy project scope definitions

struct btf_blackboard_data btfdt_app_blackboard_table[] = {

    [BTFDT_KEY_BUTTON_EMERGENCY]       = {.data      = &btfdt_button_emergency,
                                          .size      = sizeof(btfdt_button_emergency),
                                        //   .validator = NULL,
                                          .checked   = false},
    [BTFDT_KEY_HAS_EMERGENCY_OCCURRED] = {.data = &btfdt_has_emergency_occurred,
                                          .size = sizeof(
                                              btfdt_has_emergency_occurred),
                                        //   .validator = NULL,
                                          .checked   = false},
    [BTFDT_KEY_DOOR_STATUS]            = {.data      = &btfdt_door_status,
                                          .size      = sizeof(btfdt_door_status),
                                        //   .validator = NULL,
                                          .checked   = false},
    [BTFDT_KEY_MOTOR_STATUS]           = {.data      = &btfdt_motor_status,
                                          .size      = sizeof(btfdt_motor_status),
                                        //   .validator = NULL,
                                          .checked   = false},
};

int n = sizeof(btfdt_door_status);

// struct btf_blackboard btfdt_app_blackboard