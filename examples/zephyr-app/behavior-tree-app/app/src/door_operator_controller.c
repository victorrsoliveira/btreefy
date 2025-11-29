/**
 * @file door_operator_controller.c
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-06-17
 *
 */

#include "door_operator_controller.h"

#include <zephyr/kernel.h>

static char *door_sensor_status_string[] = {[DOOR_IS_UNDEFINED] = "UNDEFINED",
                                            [DOOR_IS_OPEN]      = "OPEN",
                                            [DOOR_IS_CLOSED]    = "CLOSED"};

static struct
{
    enum door_action        opener_action;
    enum door_sensor_status sensor_status;
} door_ctrl;

struct k_timer door_sensor_timer;
static void    door_sensor_timer_cb(struct k_timer *timer_id);

int door_operator_ctrl_init(void)
{
    door_ctrl.opener_action = STOP_DOOR;
    door_ctrl.sensor_status = DOOR_IS_CLOSED;

    k_timer_init(&door_sensor_timer, door_sensor_timer_cb, NULL);
    return 0;
}

enum door_sensor_status door_operator_ctrl_get_sensor_status(void)
{
    return door_ctrl.sensor_status;
}

enum door_action door_operator_ctrl_get_opener_action(void)
{
    return door_ctrl.opener_action;
}

char *door_operator_ctrl_get_sensor_status_string(
    enum door_sensor_status status)
{
    return door_sensor_status_string[status];
}

int door_operator_ctrl_set_action(enum door_action action)
{
    if (action != door_ctrl.opener_action)
    {
        door_ctrl.opener_action = action;

        if ((action == OPEN_DOOR) || (action == CLOSE_DOOR))
        {
            door_ctrl.sensor_status = DOOR_IS_UNDEFINED;
            k_timer_start(&door_sensor_timer, K_MSEC(4000), K_NO_WAIT);
        }
        else
        {
            k_timer_stop(&door_sensor_timer);
        }
    }

    return 0;
}

static void door_sensor_timer_cb(struct k_timer *timer_id)
{
    printf("Door sensor timer callback\n");

    if (door_ctrl.opener_action == OPEN_DOOR)
    {
        door_ctrl.sensor_status = DOOR_IS_OPEN;
    }
    else if (door_ctrl.opener_action == CLOSE_DOOR)
    {
        door_ctrl.sensor_status = DOOR_IS_CLOSED;
    }
    else
    {
        // NEVER OCCURS
    }
}
