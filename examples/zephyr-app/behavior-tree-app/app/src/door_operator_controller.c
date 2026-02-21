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
#include "btreefy/btf_blackboard.h"
#include "app_blackboard.h"

static char *door_sensor_status_string[] = {[DOOR_IS_UNDEFINED] = "UNDEFINED",
                                            [DOOR_IS_OPEN]      = "OPEN",
                                            [DOOR_IS_CLOSED]    = "CLOSED"};


struct k_timer door_sensor_timer;
static void    door_sensor_timer_cb(struct k_timer *timer_id);

BTF_BLACKBOARD_DECLARE(app_blackboard, struct app_blackboard);


int door_operator_ctrl_init(void)
{
    k_timer_init(&door_sensor_timer, door_sensor_timer_cb, NULL);
    return 0;
}

char *door_operator_ctrl_get_sensor_status_string(
    enum door_sensor_status status)
{
    return door_sensor_status_string[status];
}

int door_operator_ctrl_set_action(enum door_action action)
{
    enum btfdt_door_sensor_status door_status;
    enum btfdt_motor_action_status motor_status;

    BTF_BLACKBOARD_RETRIEVE_DATA(app_blackboard,  motor_status, motor_status);

    if ((enum btfdt_motor_action_status) action != motor_status)
    {
        BTF_BLACKBOARD_UPDATE_DATA(app_blackboard,  motor_status, action);

        if ((action == OPEN_DOOR) || (action == CLOSE_DOOR))
        {
            door_status = BTFDT_DOOR_IS_UNDEFINED;
            BTF_BLACKBOARD_UPDATE_DATA(app_blackboard,  door_status, door_status);
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
    enum btfdt_door_sensor_status door_status;
    enum btfdt_motor_action_status motor_status;

    printf("Door sensor timer callback\n");

    BTF_BLACKBOARD_RETRIEVE_DATA(app_blackboard,  motor_status, motor_status);
    if (motor_status == BTFDT_MOTOR_OPEN_DOOR)
    {
        door_status = BTFDT_DOOR_IS_OPEN;
        BTF_BLACKBOARD_UPDATE_DATA(app_blackboard,  door_status, door_status);
    }
    else if (motor_status == BTFDT_MOTOR_CLOSE_DOOR)
    {
        door_status = BTFDT_DOOR_IS_CLOSED;
        BTF_BLACKBOARD_UPDATE_DATA(app_blackboard,  door_status, door_status);
    }
    else
    {
        // NEVER OCCURS
    }
}
