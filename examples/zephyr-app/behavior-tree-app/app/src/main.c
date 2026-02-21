/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>
#include <stdio.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#include "btreefy/btf_tree_runner.h"
#include "btreefy/btreefy.h"
#include "door_operator_controller.h"

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/*
 * Get button configuration from the devicetree sw0 alias. This is mandatory.
 */
#define SW0_NODE DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif

extern struct btf_node nodes[];
extern size_t          nodes_size;

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

static const struct gpio_dt_spec button =
    GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static struct gpio_callback button_cb_data;

static btf_node_status_t mock_open_door_action_status  = BTF_SUCCESS_STATUS;
static btf_node_status_t mock_close_door_action_status = BTF_SUCCESS_STATUS;
static btf_node_status_t mock_stop_door_action_status  = BTF_SUCCESS_STATUS;

static bool    button_pressed_flag     = false;
static bool    open_door_request_flag  = false;
static bool    close_door_request_flag = false;
static bool    has_emergency_occurred  = false;
static uint8_t button_pressed_count    = 0;

static struct k_timer button_timer;
static void           button_timer_cb(struct k_timer *timer_id);

void button_pressed(const struct device *dev, struct gpio_callback *cb,
                    uint32_t pins)
{
    static int64_t last_button_pressed_evt_time = 0;

    int64_t current_time = k_uptime_get();

    if (current_time - last_button_pressed_evt_time > 50)
    {
        last_button_pressed_evt_time = current_time;
        if (button_pressed_count == 0)
        {
            k_timer_start(&button_timer, K_MSEC(800), K_NO_WAIT);
        }
        button_pressed_count++;
    }
}

void button_timer_cb(struct k_timer *timer_id)
{
    bool is_valid = true;

    switch (button_pressed_count)
    {
    case 1:
    {
        button_pressed_flag = true;
    }
    break;

    case 2:
    {
        open_door_request_flag = true;
    }
    break;

    case 3:
    {
        close_door_request_flag = true;
    }
    break;

    default:
        is_valid = false;
        break;
    }

    if (is_valid)
    {
        btf_runner_notify_event(BTF_RUNNER_TICK_REQUEST_EVT);
    }

    button_pressed_count = 0;
}

// ### BT ACTIONS - START ###

btf_node_status_t stop_door_action(btf_tree_st *tree, void *data,
                                   size_t datalen)
{
    printf("Door has stopped.\n");
    door_operator_ctrl_set_action(STOP_DOOR);

    return BTF_SUCCESS_STATUS;
}

btf_node_status_t open_door_action(btf_tree_st *tree, void *data,
                                   size_t datalen)
{
    printf("Door is opening... ");
    if (door_operator_ctrl_get_sensor_status() != DOOR_IS_OPEN)
    {
        printf(" request ACCEPTED!\n");
        door_operator_ctrl_set_action(OPEN_DOOR);
    }
    else
    {
        printf(" but is alreaedy OPEN, then REJECTED!\n");
    }

    return BTF_SUCCESS_STATUS;
}

btf_node_status_t close_door_action(btf_tree_st *tree, void *data,
                                    size_t datalen)
{
    printf("Door is closing... ");
    door_operator_ctrl_set_action(CLOSE_DOOR);
    return mock_close_door_action_status;
}

btf_node_status_t emergency_action(btf_tree_st *tree, void *data,
                                   size_t datalen)
{
    printf("Emergency occurred!\n");
    has_emergency_occurred = true;
    btf_runner_notify_event(BTF_RUNNER_TICK_REQUEST_EVT);
    return BTF_SUCCESS_STATUS;
}

// ### BT ACTIONS - END ###

// ### BT CONDITIONS - START ###

btf_node_status_t is_motor_on_cond(btf_tree_st *tree, void *data,
                                   size_t datalen)
{
    return BTF_SUCCESS_STATUS;
}

btf_node_status_t is_emerg_btn_pressed_cond(btf_tree_st *tree, void *data,
                                            size_t datalen)
{
    btf_node_status_t ret = BTF_FAILURE_STATUS;

    if (button_pressed_flag)
    {
        printf("Emergency button pressed!\n");
        button_pressed_flag = false;
        ret                 = BTF_SUCCESS_STATUS;
    }
    return ret;
}

btf_node_status_t is_open_cond(btf_tree_st *tree, void *data, size_t datalen)
{
    btf_node_status_t ret = BTF_FAILURE_STATUS;

    if (door_operator_ctrl_get_sensor_status() == DOOR_IS_OPEN)
    {
        ret = BTF_SUCCESS_STATUS;
        printf("Door is open\n");
    }
    else if (door_operator_ctrl_get_sensor_status() == DOOR_IS_CLOSED)
    {
        printf("Door is closed\n");
    }

    return ret;
}

btf_node_status_t is_opening_cond(btf_tree_st *tree, void *data, size_t datalen)
{
    btf_node_status_t ret = BTF_FAILURE_STATUS;

    if ((door_operator_ctrl_get_sensor_status() == DOOR_IS_UNDEFINED)
        && (door_operator_ctrl_get_opener_action() == OPEN_DOOR))
    {
        ret = BTF_SUCCESS_STATUS;
    }

    return ret;
}

btf_node_status_t is_closed_cond(btf_tree_st *tree, void *data, size_t datalen)
{
    btf_node_status_t ret = BTF_FAILURE_STATUS;

    if (door_operator_ctrl_get_sensor_status() == DOOR_IS_CLOSED)
    {
        ret = BTF_SUCCESS_STATUS;
        printf("Door is closed\n");
    }
    else if (door_operator_ctrl_get_sensor_status() == DOOR_IS_OPEN)
    {
        printf("Door is open\n");
    }

    return ret;
}

btf_node_status_t is_closing_cond(btf_tree_st *tree, void *data, size_t datalen)
{
    btf_node_status_t ret = BTF_FAILURE_STATUS;

    if ((door_operator_ctrl_get_sensor_status() == DOOR_IS_UNDEFINED)
        && (door_operator_ctrl_get_opener_action() == CLOSE_DOOR))
    {
        ret = BTF_SUCCESS_STATUS;
    }

    return ret;
}

btf_node_status_t has_emergency_ocurred_cond(btf_tree_st *tree, void *data,
                                             size_t datalen)
{
    btf_node_status_t ret = BTF_FAILURE_STATUS;

    if (has_emergency_occurred)
    {
        has_emergency_occurred = false;
        ret                    = BTF_SUCCESS_STATUS;
    }

    return ret;
}

btf_node_status_t open_door_request_cond(btf_tree_st *tree, void *data,
                                         size_t datalen)
{
    btf_node_status_t ret = BTF_FAILURE_STATUS;

    if (open_door_request_flag)
    {
        printf("Open door request!\n");
        open_door_request_flag = false;
        ret                    = BTF_SUCCESS_STATUS;
    }

    return ret;
}

btf_node_status_t close_door_request_cond(btf_tree_st *tree, void *data,
                                          size_t datalen)
{
    btf_node_status_t ret = BTF_FAILURE_STATUS;

    if (close_door_request_flag)
    {
        printf("Close door request!\n");
        close_door_request_flag = false;
        ret                     = BTF_SUCCESS_STATUS;
    }

    return ret;
}

// ### BT CONDITIONS - END ###


int main(void)
{
    int  ret;
    bool led_state = true;

    btf_tree_st tree;
    int32_t     status;

    if (!gpio_is_ready_dt(&led))
    {
        return 0;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (ret < 0)
    {
        return 0;
    }

    if (!gpio_is_ready_dt(&button))
    {
        printk("Error: button device %s is not ready\n", button.port->name);
        return 0;
    }

    k_timer_init(&button_timer, button_timer_cb, NULL);

    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret != 0)
    {
        printk("Error %d: failed to configure %s pin %d\n", ret,
               button.port->name, button.pin);
        return 0;
    }

    ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0)
    {
        printk("Error %d: failed to configure interrupt on %s pin %d\n", ret,
               button.port->name, button.pin);
        return 0;
    }

    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);
    printk("Set up button at %s pin %d\n", button.port->name, button.pin);

    if (door_operator_ctrl_init())
    {
        return 0;
    }

    if (btf_init(&tree, nodes, nodes_size) == 0)
    {
    }

    if (btf_runner_init(&tree))
    {
    }

    while (1)
    {
        k_msleep(1000);
    }

    return 0;
}