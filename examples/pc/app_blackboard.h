/**
 * @file app_blackboard.h
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief
 * @version 0.1
 * @date 2026-01-27
 *
 */

#ifndef APP_BLACKBOARD_H
#define APP_BLACKBOARD_H

#include <stdbool.h>
#include <pthread.h>

enum btfdt_door_sensor_status
{
    BTFDT_DOOR_IS_UNDEFINED,
    BTFDT_DOOR_IS_OPEN,
    BTFDT_DOOR_IS_CLOSED
};

enum btfdt_motor_action_status
{
    BTFDT_MOTOR_OPEN_DOOR  = -1,
    BTFDT_MOTOR_STOP_DOOR  = 0,
    BTFDT_MOTOR_CLOSE_DOOR = 1
};

struct app_blackboard
{
    bool                           button_emergency;
    bool                           open_request;
    bool                           close_request;
    bool                           has_emergency_occurred;
    enum btfdt_door_sensor_status  door_status;
    enum btfdt_motor_action_status motor_status;
};

extern struct app_blackboard app_blackboard_ctx;
extern pthread_mutex_t app_blackboard_mutex;

#endif  // APP_BLACKBOARD_H
