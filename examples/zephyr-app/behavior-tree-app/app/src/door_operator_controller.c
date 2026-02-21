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

#include "btreefy/btf_tree_runner.h"

static char *door_sensor_status_string[] = {[DOOR_IS_UNDEFINED] = "UNDEFINED",
                                            [DOOR_IS_OPEN]      = "OPEN",
                                            [DOOR_IS_CLOSED]    = "CLOSED"};

static enum door_sensor_status  door_sensor_status = DOOR_IS_OPEN;
static enum door_action         opener_action      = STOP_DOOR;

struct k_timer door_sensor_timer;
static void    door_sensor_timer_cb(struct k_timer *timer_id);

int door_operator_ctrl_init(void)
{
    k_timer_init(&door_sensor_timer, door_sensor_timer_cb, NULL);
    return 0;
}

enum door_sensor_status door_operator_ctrl_get_sensor_status(void)
{
    return door_sensor_status;
}

enum door_action door_operator_ctrl_get_opener_action(void)
{
    return opener_action;
}

char *door_operator_ctrl_get_sensor_status_string(
    enum door_sensor_status status)
{
    return door_sensor_status_string[status];
}

int door_operator_ctrl_set_action(enum door_action action)
{
    if (action != opener_action)
    {
        opener_action = action;

        if ((action == OPEN_DOOR) || (action == CLOSE_DOOR))
        {
            door_sensor_status = DOOR_IS_UNDEFINED;
            k_timer_start(&door_sensor_timer, K_MSEC(4000), K_NO_WAIT);
        }
        else
        {
            k_timer_stop(&door_sensor_timer);
        }
        btf_runner_notify_event(BTF_RUNNER_TICK_REQUEST_EVT);
    }

    return 0;
}

static void door_sensor_timer_cb(struct k_timer *timer_id)
{
    printf("Door sensor timer callback\n");

    if (opener_action == OPEN_DOOR)
    {
        door_sensor_status = DOOR_IS_OPEN;
        btf_runner_notify_event(BTF_RUNNER_TICK_REQUEST_EVT);
    }
    else if (opener_action == CLOSE_DOOR)
    {
        door_sensor_status = DOOR_IS_CLOSED;
        btf_runner_notify_event(BTF_RUNNER_TICK_REQUEST_EVT);
    }
    else
    {
        // NEVER OCCURS
    }
}
