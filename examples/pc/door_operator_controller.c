/**
 * @file door_operator_controller.c
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-06-17
 *
 */

#include "door_operator_controller.h"

#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "btreefy/btf_blackboard.h"
#include "app_blackboard.h"

static char *door_sensor_status_string[] = {[DOOR_IS_UNDEFINED] = "UNDEFINED",
                                            [DOOR_IS_OPEN]      = "OPEN",
                                            [DOOR_IS_CLOSED]    = "CLOSED"};

static pthread_t      door_timer_thread;
static pthread_mutex_t door_mutex    = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  door_cond     = PTHREAD_COND_INITIALIZER;
static bool            timer_active  = false;
static bool            stop_timer    = false;

BTF_BLACKBOARD_DECLARE(app_blackboard, struct app_blackboard);

static void *door_timer_thread_fn(void *arg);

int door_operator_ctrl_init(void)
{
    return pthread_create(&door_timer_thread, NULL, door_timer_thread_fn, NULL);
}

char *door_operator_ctrl_get_sensor_status_string(
    enum door_sensor_status status)
{
    return door_sensor_status_string[status];
}

int door_operator_ctrl_set_action(enum door_action action)
{
    enum btfdt_door_sensor_status  door_status;
    enum btfdt_motor_action_status motor_status;

    BTF_BLACKBOARD_RETRIEVE_DATA(app_blackboard, motor_status, motor_status);

    if ((enum btfdt_motor_action_status) action != motor_status)
    {
        BTF_BLACKBOARD_UPDATE_DATA(app_blackboard, motor_status, action);

        pthread_mutex_lock(&door_mutex);
        if ((action == OPEN_DOOR) || (action == CLOSE_DOOR))
        {
            door_status = BTFDT_DOOR_IS_UNDEFINED;
            BTF_BLACKBOARD_UPDATE_DATA(app_blackboard, door_status, door_status);
            timer_active = true;
            pthread_cond_signal(&door_cond);
        }
        else
        {
            timer_active = false;
        }
        pthread_mutex_unlock(&door_mutex);
    }

    return 0;
}

static void *door_timer_thread_fn(void *arg)
{
    enum btfdt_door_sensor_status  door_status;
    enum btfdt_motor_action_status motor_status;

    while (!stop_timer)
    {
        pthread_mutex_lock(&door_mutex);
        while (!timer_active && !stop_timer)
        {
            pthread_cond_wait(&door_cond, &door_mutex);
        }
        pthread_mutex_unlock(&door_mutex);

        if (stop_timer)
        {
            break;
        }

        usleep(4000000); // 4 second door travel time

        pthread_mutex_lock(&door_mutex);
        if (timer_active)
        {
            printf("Door sensor timer callback\n");
            BTF_BLACKBOARD_RETRIEVE_DATA(app_blackboard, motor_status,
                                         motor_status);
            if (motor_status == BTFDT_MOTOR_OPEN_DOOR)
            {
                door_status = BTFDT_DOOR_IS_OPEN;
                BTF_BLACKBOARD_UPDATE_DATA(app_blackboard, door_status,
                                           door_status);
            }
            else if (motor_status == BTFDT_MOTOR_CLOSE_DOOR)
            {
                door_status = BTFDT_DOOR_IS_CLOSED;
                BTF_BLACKBOARD_UPDATE_DATA(app_blackboard, door_status,
                                           door_status);
            }
            timer_active = false;
        }
        pthread_mutex_unlock(&door_mutex);
    }

    return NULL;
}
