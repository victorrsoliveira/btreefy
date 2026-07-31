/**
 * @file btf_blackboard.c
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-08-11
 *
 */

#include "btreefy/btf_blackboard.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "btreefy/btreefy_objs.h"

int32_t btf_blackboard_retrieve_data(struct btf_blackboard *blackboard,
                                     size_t offset, void *data, size_t size)
{
    assert(data != NULL);

    memcpy(data, (uint8_t *) blackboard->data + offset, size);

    return 0;
}

int32_t btf_blackboard_update_data(struct btf_blackboard *blackboard,
                                   size_t offset, void *data, size_t size)
{
    assert(data != NULL);

    memcpy((uint8_t *) blackboard->data + offset, data, size);

    return 0;
}
