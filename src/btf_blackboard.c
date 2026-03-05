/**
 * @file btf_blackboard.c
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-08-11
 *
 */

#include "btreefy/btf_blackboard.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "btreefy/btf_tree_runner.h"
#include "btreefy/btreefy_objs.h"

int32_t btf_blackboard_retrieve_data(struct btf_blackboard *blackboard,
                                     size_t offset, void *data, size_t size)
{
    assert(data != NULL);

    // TODO: Add bounds checking here: if (offset + size > blackboard->capacity)
    // return error;

    memcpy(data, (uint8_t *) blackboard->data + offset, size);

    return 0;
}

int32_t btf_blackboard_update_data(struct btf_blackboard *blackboard,
                                   size_t offset, void *data, size_t size)
{
    assert(data != NULL);

    // TODO: Add bounds checking here: if (offset + size > blackboard->capacity)

    memcpy((uint8_t *) blackboard->data + offset, data, size);

    if (blackboard->notify_cb != NULL)
    {
        blackboard->notify_cb(blackboard->notify_context);
    }

    return 0;
}

void btf_blackboard_set_notify_cb(struct btf_blackboard     *blackboard,
                                  btf_blackboard_notify_cb_t notify_cb,
                                  void                      *notify_context)
{
    blackboard->notify_cb = notify_cb;
    blackboard->notify_context = notify_context;
}