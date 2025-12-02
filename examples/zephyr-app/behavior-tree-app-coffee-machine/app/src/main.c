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

static bool make_coffee_request_flag     = false;
static bool overheat_flag                = false;
static bool water_level_low_flag         = false;
static bool idle_flag                    = false;
static bool self_test_flag               = false;
static bool heating_flag                 = false;
static bool rinsing_flag                 = false;
static int  water_level_low_action_count = 0;

static uint8_t        button_pressed_count = 0;
static struct k_timer button_timer;
static void           button_timer_cb(struct k_timer *timer_id);

void button_pressed(const struct device *dev, struct gpio_callback *cb,
                    uint32_t pins)
{
    static int64_t last_button_pressed_evt_time = 0;

    int64_t current_time = k_uptime_get();

    if (current_time - last_button_pressed_evt_time > 50)
    {
        // printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
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
    switch (button_pressed_count)
    {
    case 1:
    {
        make_coffee_request_flag = true;
    }
    break;

    case 2:
    {
        water_level_low_flag = !water_level_low_flag;
        printf("Water level is %s\n", water_level_low_flag ? "low" : "normal");
    }
    break;

    case 3:
    {
        overheat_flag = true;
    }
    break;

    default:
        break;
    }

    button_pressed_count = 0;
}

// BTF FUNCTIONS - START

btf_node_status_t run_self_test(btf_tree_st *tree, void *data, size_t datalen)
{
    static int n = 0;

    if (n == 3)
    {
        n              = 0;
        self_test_flag = true;
        return BTF_SUCCESS_STATUS;
    }

    n++;
    return BTF_RUNNING_STATUS;
}

btf_node_status_t prepare_coffee(btf_tree_st *tree, void *data, size_t datalen)
{
    static int n = 0;

    if (n == 3)
    {
        n                        = 0;
        make_coffee_request_flag = false;
        return BTF_SUCCESS_STATUS;
    }

    n++;
    return BTF_RUNNING_STATUS;
}

btf_node_status_t run_rinsing(btf_tree_st *tree, void *data, size_t datalen)
{
    static int n = 0;

    if (n == 3)
    {
        n            = 0;
        rinsing_flag = true;
        return BTF_SUCCESS_STATUS;
    }

    n++;
    return BTF_RUNNING_STATUS;
}

btf_node_status_t is_idle_condition(btf_tree_st *tree, void *data,
                                    size_t datalen)
{
    return idle_flag ? BTF_SUCCESS_STATUS : BTF_FAILURE_STATUS;
}

btf_node_status_t is_heating_done_condition(btf_tree_st *tree, void *data,
                                            size_t datalen)
{
    return heating_flag ? BTF_SUCCESS_STATUS : BTF_FAILURE_STATUS;
}

btf_node_status_t run_heating(btf_tree_st *tree, void *data, size_t datalen)
{
    static int n = 0;

    if (n == 3)
    {
        n            = 0;
        heating_flag = true;
        return BTF_SUCCESS_STATUS;
    }

    n++;
    return BTF_RUNNING_STATUS;
}

btf_node_status_t overheat_handler(btf_tree_st *tree, void *data,
                                   size_t datalen)
{
    return BTF_SUCCESS_STATUS;
}

btf_node_status_t is_overheat_condition(btf_tree_st *tree, void *data,
                                        size_t datalen)
{
    return overheat_flag ? BTF_SUCCESS_STATUS : BTF_FAILURE_STATUS;
}

btf_node_status_t set_idle_state(btf_tree_st *tree, void *data, size_t datalen)
{
    idle_flag = true;
    return BTF_SUCCESS_STATUS;
}

btf_node_status_t is_rinsing_done_condition(btf_tree_st *tree, void *data,
                                            size_t datalen)
{
    return rinsing_flag ? BTF_SUCCESS_STATUS : BTF_FAILURE_STATUS;
}

btf_node_status_t is_self_test_success_condition(btf_tree_st *tree, void *data,
                                                 size_t datalen)
{
    return self_test_flag ? BTF_SUCCESS_STATUS : BTF_FAILURE_STATUS;
}

btf_node_status_t water_reservoir_is_low(btf_tree_st *tree, void *data,
                                         size_t datalen)
{
    if (water_level_low_action_count < 0)
    {
        water_level_low_action_count++;
        printf("ACTION: Water level reservoir capacity is running low!\n");
    }
    return BTF_SUCCESS_STATUS;
}

btf_node_status_t is_water_level_low_condition(btf_tree_st *tree, void *data,
                                               size_t datalen)
{
    btf_node_status_t ret = BTF_FAILURE_STATUS;

    if (water_level_low_flag)
    {
        ret = BTF_SUCCESS_STATUS;
    }
    else
    {
        water_level_low_action_count = 0;
    }
    return ret;
}

btf_node_status_t wait_coffee_request(btf_tree_st *tree, void *data,
                                      size_t datalen)
{
    return make_coffee_request_flag ? BTF_SUCCESS_STATUS : BTF_RUNNING_STATUS;
}

// BTF FUNCTIONS - END

int main(void)
{
    int ret;

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

    if (btf_init(&tree, nodes, nodes_size) == 0)
    {
    }

    while (1)
    {
        status = btf_tick_tree(&tree);
        printf("Tree executed and returned %s\n",
               btf_global_action_status_string[status]);
        printf("-------------------------------------------\n");

        k_msleep(1000);
    }

    return 0;
}