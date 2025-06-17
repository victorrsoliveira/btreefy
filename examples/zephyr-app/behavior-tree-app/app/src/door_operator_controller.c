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

static char *door_sensor_status_string[] = {
    [DOOR_IS_OPEN]    = "OPEN",
    [DOOR_IS_CLOSED]  = "CLOSED",
    [DOOR_IS_OPENING] = "OPENING",
    [DOOR_IS_CLOSING] = "CLOSING",
};

static struct
{
    enum door_action        opener_state;
    enum door_sensor_status sensor_status;
} door_ctrl;

struct k_timer door_sensor_timer;
static void    door_sensor_timer_cb(struct k_timer *timer_id);

int door_operator_ctrl_init(void)
{
    door_ctrl.opener_state  = STOP_DOOR;
    door_ctrl.sensor_status = DOOR_IS_CLOSED;

    k_timer_init(&door_sensor_timer, door_sensor_timer_cb, NULL);
    return 0;
}

enum door_sensor_status door_operator_ctrl_get_status(void)
{
    enum door_sensor_status ret = door_ctrl.sensor_status;

    if (door_ctrl.opener_state == OPEN_DOOR)
    {
        ret = DOOR_IS_OPENING;
    }
    else if (door_ctrl.opener_state == CLOSE_DOOR)
    {
        ret = DOOR_IS_CLOSING;
    }
    else
    {
        // Empty
    }

    return ret;
}

char *door_operator_ctrl_get_status_string(enum door_sensor_status status)
{
    return door_sensor_status_string[status];
}

int door_operator_ctrl_set_action(enum door_action action)
{
    if (action != door_ctrl.opener_state)
    {
        door_ctrl.opener_state = action;

        if ((action == OPEN_DOOR) || (action == CLOSE_DOOR))
        {
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

    switch (door_operator_ctrl_get_status())
    {
    case DOOR_IS_CLOSING:
    {
        door_ctrl.sensor_status = DOOR_IS_CLOSED;
        printf("Door is closed.\n");
    }
    break;

    case DOOR_IS_OPENING:
    {
        door_ctrl.sensor_status = DOOR_IS_OPEN;
        printf("Door is open.\n");
    }
    break;

    default:
        /* code */
        break;
    }
}