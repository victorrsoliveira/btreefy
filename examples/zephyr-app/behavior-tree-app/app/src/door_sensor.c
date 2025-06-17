/**
 * @file door_sensor.c
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-06-15
 *
 */

#include "door_sensor.h"

#include <zephyr/kernel.h>

#define DOOR_SENSOR_STATUS_INITIAL_VALUE DOOR_IS_CLOSED

static char *door_sensor_status_string[] = {
    [DOOR_IS_OPEN]    = "OPEN",
    [DOOR_IS_CLOSED]  = "CLOSED",
    [DOOR_IS_OPENING] = "OPENING",
    [DOOR_IS_CLOSING] = "CLOSING",
};


static enum door_sensor_status door_sensor_status =
    DOOR_SENSOR_STATUS_INITIAL_VALUE;

struct k_timer door_sensor_timer;
void           door_sensor_timer_cb(struct k_timer *timer_id);

int bt_app_door_sensor_init(void)
{
    door_sensor_status = DOOR_SENSOR_STATUS_INITIAL_VALUE;

    k_timer_init(&door_sensor_timer, door_sensor_timer_cb, NULL);

    return 0;
}

enum door_sensor_status bt_app_door_sensor_get_status(void)
{
    return door_sensor_status;
}

char * bt_app_door_sensor_get_status_string(enum door_sensor_status status)
{
    return door_sensor_status_string[status];
}

int bt_app_door_sensor_set_status(enum door_sensor_status status)
{
    switch (door_sensor_status)
    {
    case DOOR_IS_OPEN:
    {
        if (status == DOOR_IS_CLOSING)
        {
            door_sensor_status = status;
            k_timer_start(&door_sensor_timer, K_MSEC(4000), K_NO_WAIT);
        }
    }
    break;

    case DOOR_IS_CLOSED:
    {
        if (status == DOOR_IS_OPENING)
        {
            door_sensor_status = status;
            k_timer_start(&door_sensor_timer, K_MSEC(4000), K_NO_WAIT);
        }
    }
    break;

    case DOOR_IS_OPENING:
    {
    }
    break;

    case DOOR_IS_CLOSING:
    {
    }
    break;

    default:
    {
    }
    break;
    }

    return 0;
}

void door_sensor_timer_cb(struct k_timer *timer_id)
{
    switch (door_sensor_status)
    {
    case DOOR_IS_CLOSING:
    {
        door_sensor_status = DOOR_IS_CLOSED;
        printf("Door is closed.\n");
    }
    break;

    case DOOR_IS_OPENING:
    {
        door_sensor_status = DOOR_IS_OPEN;
        printf("Door is open.\n");
    }
    break;

    default:
        /* code */
        break;
    }
}