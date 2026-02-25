/**
 * @file door_operator_controller.h
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-06-17
 *
 */

#ifndef DOOR_OPERATOR_CONTROLLER_H
#define DOOR_OPERATOR_CONTROLLER_H

#include "app_blackboard.h"

enum door_action
{
    OPEN_DOOR  = BTFDT_MOTOR_OPEN_DOOR,
    STOP_DOOR  = BTFDT_MOTOR_STOP_DOOR,
    CLOSE_DOOR = BTFDT_MOTOR_CLOSE_DOOR
};

enum door_sensor_status
{
    DOOR_IS_UNDEFINED,
    DOOR_IS_OPEN,
    DOOR_IS_CLOSED
};

int door_operator_ctrl_init(void);

char *door_operator_ctrl_get_sensor_status_string(
    enum door_sensor_status status);

int door_operator_ctrl_set_action(enum door_action action);

#endif  // DOOR_OPERATOR_CONTROLLER_H
