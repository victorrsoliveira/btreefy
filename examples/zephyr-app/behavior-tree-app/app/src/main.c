/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>
#include <stdio.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#include "btreefy/btreefy.h"
#include "door_sensor.h"

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

static bool button_pressed_flag    = false;
static bool has_emergency_occurred = false;

void button_pressed(const struct device *dev, struct gpio_callback *cb,
                    uint32_t pins)
{
    static int64_t last_button_pressed_evt_time = 0;

    int64_t current_time = k_uptime_get();

    if (current_time - last_button_pressed_evt_time > 10000)
    {
        printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
        last_button_pressed_evt_time = current_time;
        button_pressed_flag          = true;
    }
}

// ### BT ACTIONS - START ###

btf_node_status_t stop_door_action(btf_tree_st *tree, void *data,
                                   size_t datalen)
{
    printf("Door has stopped.\n");
    return BTF_SUCCESS_STATUS;
}

btf_node_status_t open_door_action(btf_tree_st *tree, void *data,
                                   size_t datalen)
{
    printf("Door is opening... ");
    bt_app_door_sensor_set_status(DOOR_IS_OPENING);
    return BTF_SUCCESS_STATUS;
}

btf_node_status_t close_door_action(btf_tree_st *tree, void *data,
                                    size_t datalen)
{
    printf("Door is closing... ");
    bt_app_door_sensor_set_status(DOOR_IS_CLOSING);
    return mock_close_door_action_status;
}

btf_node_status_t emergency_action(btf_tree_st *tree, void *data,
                                   size_t datalen)
{
    has_emergency_occurred = true;
    return BTF_SUCCESS_STATUS;
}

// ### BT ACTIONS - END ###

// ### BT CONDITIONS - START ###

btf_node_status_t is_emerg_btn_pressed_cond(btf_tree_st *tree, void *data,
                                            size_t datalen)
{
    btf_node_status_t ret = BTF_FAILURE_STATUS;

    if (button_pressed_flag)
    {
        button_pressed_flag = false;
        ret                 = BTF_SUCCESS_STATUS;
    }
    return ret;
}

btf_node_status_t is_open_cond(btf_tree_st *tree, void *data, size_t datalen)
{
    btf_node_status_t ret = BTF_FAILURE_STATUS;

    if (bt_app_door_sensor_get_status() == DOOR_IS_OPEN)
    {
        ret = BTF_SUCCESS_STATUS;
    }

    return ret;
}

btf_node_status_t is_opening_cond(btf_tree_st *tree, void *data, size_t datalen)
{
    btf_node_status_t ret = BTF_FAILURE_STATUS;

    if (bt_app_door_sensor_get_status() == DOOR_IS_OPENING)
    {
        ret = BTF_SUCCESS_STATUS;
    }

    return ret;
}

btf_node_status_t is_closed_cond(btf_tree_st *tree, void *data, size_t datalen)
{
    btf_node_status_t ret = BTF_FAILURE_STATUS;

    if (bt_app_door_sensor_get_status() == DOOR_IS_CLOSED)
    {
        ret = BTF_SUCCESS_STATUS;
    }

    return ret;
}

btf_node_status_t is_closing_cond(btf_tree_st *tree, void *data, size_t datalen)
{
    btf_node_status_t ret = BTF_FAILURE_STATUS;

    if (bt_app_door_sensor_get_status() == DOOR_IS_CLOSING)
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
    return BTF_FAILURE_STATUS;
}

btf_node_status_t close_door_request_cond(btf_tree_st *tree, void *data,
                                          size_t datalen)
{
    return BTF_FAILURE_STATUS;
}

// ### BT CONDITIONS - END ###


struct btf_node nodes[21] = {

    [0]  = {.status  = BTF_UNDEF_STATUS,
            .parent  = BTF_NULL_NODE,
            .child   = 1,
            .sibling = BTF_NULL_NODE,
            .action  = NULL,
            .control = btf_fallback_policy_fn,
            .name    = "Fallback 1"},
    [1]  = {.status  = BTF_UNDEF_STATUS,
            .parent  = 0,
            .child   = 2,
            .sibling = 10,
            .action  = NULL,
            .control = btf_sequence_policy_fn,
            .name    = "Sequence_1"},
    [2]  = {.status  = BTF_UNDEF_STATUS,
            .parent  = 1,
            .child   = BTF_NULL_NODE,
            .sibling = 3,
            .action  = is_emerg_btn_pressed_cond,
            .control = NULL,
            .name    = "Emerg Btn?"},
    [3]  = {.status  = BTF_UNDEF_STATUS,
            .parent  = 1,
            .child   = BTF_NULL_NODE,
            .sibling = 4,
            .action  = stop_door_action,
            .control = NULL,
            .name    = "Stop door"},
    [4]  = {.status  = BTF_UNDEF_STATUS,
            .parent  = 1,
            .child   = BTF_NULL_NODE,
            .sibling = BTF_NULL_NODE,
            .action  = emergency_action,
            .control = NULL,
            .name    = "Emergency"},
    [5]  = {.status  = BTF_UNDEF_STATUS,
            .parent  = 0,
            .child   = 6,
            .sibling = 16,
            .action  = NULL,
            .control = btf_sequence_policy_fn,
            .name    = "Sequence_2"},
    [6]  = {.status  = BTF_UNDEF_STATUS,
            .parent  = 5,
            .child   = 7,
            .sibling = 9,
            .action  = NULL,
            .control = btf_fallback_policy_fn,
            .name    = "Fallback_7"},
    [7]  = {.status  = BTF_UNDEF_STATUS,
            .parent  = 6,
            .child   = BTF_NULL_NODE,
            .sibling = 8,
            .action  = is_open_cond,
            .control = NULL,
            .name    = "Is open?"},
    [8]  = {.status  = BTF_UNDEF_STATUS,
            .parent  = 6,
            .child   = BTF_NULL_NODE,
            .sibling = BTF_NULL_NODE,
            .action  = is_closed_cond,
            .control = NULL,
            .name    = "Is closed?"},
    [9]  = {.status  = BTF_UNDEF_STATUS,
            .parent  = 5,
            .child   = BTF_NULL_NODE,
            .sibling = BTF_NULL_NODE,
            .action  = stop_door_action,
            .control = NULL,
            .name    = "Stop door"},
    [10] = {.status  = BTF_UNDEF_STATUS,
            .parent  = 0,
            .child   = 11,
            .sibling = 5,
            .action  = NULL,
            .control = btf_sequence_policy_fn,
            .name    = "Sequence_3"},
    [11] = {.status  = BTF_UNDEF_STATUS,
            .parent  = 10,
            .child   = 12,
            .sibling = 15,
            .action  = NULL,
            .control = btf_fallback_policy_fn,
            .name    = "Fallback_3"},
    [12] = {.status  = BTF_UNDEF_STATUS,
            .parent  = 11,
            .child   = BTF_NULL_NODE,
            .sibling = 13,
            .action  = has_emergency_ocurred_cond,
            .control = btf_sequence_policy_fn,
            .name    = "Emerg occr'd?"},
    [13] = {.status  = BTF_UNDEF_STATUS,
            .parent  = 11,
            .child   = BTF_NULL_NODE,
            .sibling = 14,
            .action  = open_door_request_cond,
            .control = NULL,
            .name    = "Open door?"},
    [14] = {.status  = BTF_UNDEF_STATUS,
            .parent  = 11,
            .child   = BTF_NULL_NODE,
            .sibling = BTF_NULL_NODE,
            .action  = is_opening_cond,
            .control = NULL,
            .name    = "Is opening?"},
    [15] = {.status  = BTF_UNDEF_STATUS,
            .parent  = 10,
            .child   = BTF_NULL_NODE,
            .sibling = BTF_NULL_NODE,
            .action  = open_door_action,
            .control = NULL,
            .name    = "Open door"},
    [16] = {.status  = BTF_UNDEF_STATUS,
            .parent  = 0,
            .child   = 17,
            .sibling = BTF_NULL_NODE,
            .action  = NULL,
            .control = btf_sequence_policy_fn,
            .name    = "Sequence_4"},
    [17] = {.status  = BTF_UNDEF_STATUS,
            .parent  = 16,
            .child   = 18,
            .sibling = 20,
            .action  = NULL,
            .control = btf_fallback_policy_fn,
            .name    = "Fallback_4"},
    [18] = {.status  = BTF_UNDEF_STATUS,
            .parent  = 17,
            .child   = BTF_NULL_NODE,
            .sibling = 19,
            .action  = close_door_request_cond,
            .control = NULL,
            .name    = "Close door?"},
    [19] = {.status  = BTF_UNDEF_STATUS,
            .parent  = 17,
            .child   = BTF_NULL_NODE,
            .sibling = BTF_NULL_NODE,
            .action  = is_closing_cond,
            .control = NULL,
            .name    = "Is closing?"},
    [20] = {.status  = BTF_UNDEF_STATUS,
            .parent  = 16,
            .child   = BTF_NULL_NODE,
            .sibling = BTF_NULL_NODE,
            .action  = close_door_action,
            .control = NULL,
            .name    = "Close door"},
};

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

    if (bt_app_door_sensor_init())
    {
        return 0;
    }

    if (btf_init(&tree, nodes, sizeof(nodes)) == 0)
    {
    }

    while (1)
    {
        printf("Door state: %s\n", bt_app_door_sensor_get_status_string(
                                       bt_app_door_sensor_get_status()));

        status = btf_tick_tree(&tree);
        printf("Tree executed and returned %s\n",
               btf_global_action_status_string[status]);
        printf("-------------------------------------------\n");

        k_msleep(1000);
    }

    return 0;
}