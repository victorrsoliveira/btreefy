/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include "btreefy/btreefy.h"

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

btf_node_status_t mock_run_ble_action_status         = BTF_SUCCESS_STATUS;
btf_node_status_t mock_run_lora_action_status        = BTF_RUNNING_STATUS;
btf_node_status_t mock_reset_variables_action_status = BTF_SUCCESS_STATUS;

btf_node_status_t reset_variables_action(btf_tree_st *tree, void *data,
                                         size_t datalen)
{
    return mock_reset_variables_action_status;
}

btf_node_status_t run_ble_action(btf_tree_st *tree, void *data, size_t datalen)
{
    return mock_run_ble_action_status;
}

btf_node_status_t run_lora_action(btf_tree_st *tree, void *data, size_t datalen)
{
    return mock_run_lora_action_status;
}

btf_node_status_t go_to_sleep_action(btf_tree_st *tree, void *data,
                                     size_t datalen)
{
    return BTF_SUCCESS_STATUS;
}

struct btf_node nodes[] = {

    [0] = {.status  = BTF_UNDEF_STATUS,
           .parent  = BTF_NULL_NODE,
           .child   = 2,
           .sibling = BTF_NULL_NODE,
           .action  = NULL,
           .control = btf_sequence_policy_fn,
           .name    = "Sequence_1"},

    [1] = {.status  = BTF_UNDEF_STATUS,
           .parent  = 0,
           .child   = 2,
           .sibling = 3,
           .action  = NULL,
           .control = btf_success_policy_fn,
           .name    = "Success_1"},

    [2] = {.status  = BTF_UNDEF_STATUS,
           .parent  = 0,
           .child   = BTF_NULL_NODE,
           .sibling = 3,
           .action  = reset_variables_action,
           .control = NULL,
           .name    = "Reset_Variables"},

    [3] = {.status  = BTF_UNDEF_STATUS,
           .parent  = 0,
           .child   = 4,
           .sibling = 6,
           .action  = NULL,
           .control = btf_sequence_policy_fn,
           .name    = "Sequence_2"},

    [4] = {.status  = BTF_UNDEF_STATUS,
           .parent  = 3,
           .child   = BTF_NULL_NODE,
           .sibling = 5,
           .action  = run_ble_action,
           .control = NULL,
           .name    = "Run_BLE"},

    [5] = {.status  = BTF_UNDEF_STATUS,
           .parent  = 3,
           .child   = BTF_NULL_NODE,
           .sibling = BTF_NULL_NODE,
           .action  = run_lora_action,
           .control = NULL,
           .name    = "Run_LoRa"},

    [6] = {.status  = BTF_UNDEF_STATUS,
           .parent  = 0,
           .child   = 7,
           .sibling = BTF_NULL_NODE,
           .action  = NULL,
           .control = btf_success_policy_fn,
           .name    = "Success_2"},

    [7] = {.status  = BTF_UNDEF_STATUS,
           .parent  = 6,
           .child   = BTF_NULL_NODE,
           .sibling = BTF_NULL_NODE,
           .action  = go_to_sleep_action,
           .control = NULL,
           .name    = "Go_To_Sleep"},
};


int main(void)
{
	int ret;
	bool led_state = true;

    btf_tree_st tree;
    int32_t     status;

	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

    if (btf_init(&tree, nodes, sizeof(nodes)) == 0)
    {
        for (size_t i = 0; i < 3; i++)
        {
            status = btf_tick_tree(&tree);
            printf("Tree executed and returned %s\n",
                   btf_global_action_status_string[status]);
            printf("-------------------------------------------\n");
            switch (i)
            {
            case 0:
                //   mock_run_ble_action_status = BTF_FAILURE_STATUS;
                mock_reset_variables_action_status = BTF_FAILURE_STATUS;
                break;

            case 1:
                mock_run_lora_action_status        = BTF_SUCCESS_STATUS;
                mock_reset_variables_action_status = BTF_SUCCESS_STATUS;
                //   mock_run_ble_action_status = BTF_SUCCESS_STATUS;
                break;

            default:
                //   mock_run_ble_action_status = BTF_SUCCESS_STATUS;
                break;
            }
        }
    }

	while (1) {
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) {
			return 0;
		}

		led_state = !led_state;
		printf("LED state: %s\n", led_state ? "ON" : "OFF");
		k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}