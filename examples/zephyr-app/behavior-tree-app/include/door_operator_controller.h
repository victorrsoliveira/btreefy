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

#include <stdint.h>

enum door_sensor_status
{
    DOOR_IS_UNDEFINED = 0,
    DOOR_IS_OPEN,
    DOOR_IS_CLOSED
};

enum door_action
{
    OPEN_DOOR  = -1,
    STOP_DOOR  = 0,
    CLOSE_DOOR = 1
};

int door_operator_ctrl_init(void);

enum door_sensor_status door_operator_ctrl_get_sensor_status(void);

enum door_action door_operator_ctrl_get_opener_action(void);

char *door_operator_ctrl_get_sensor_status_string(
    enum door_sensor_status status);

int door_operator_ctrl_set_action(enum door_action action);

#endif  // DOOR_OPERATOR_CONTROLLER_H