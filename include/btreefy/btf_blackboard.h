/**
 * @file btf_blackboard.h
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-08-11
 *
 */

#ifndef BTF_BLACKBOARD_H
#define BTF_BLACKBOARD_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "btreefy/btreefy.h"

#define BTF_UTILS_DO_CONCAT(x, y) x##y
#define BTF_UTILS_CONCAT(x, y)    BTF_UTILS_DO_CONCAT(x, y)

#define BTF_UTILS_DO_STRINGIFY(x) #x
#define BTF_UTILS_STRINGIFY(s)    BTF_UTILS_DO_STRINGIFY(s)

#define BTF_BLACKBOARD_MEMBER_SIZE(st, m) sizeof(((st *) (0))->m)

#define BTF_BLACKBOARD_INIT_VAL(...) \
    {                                \
        __VA_ARGS__                  \
    }

#define BTF_BLACKBOARD_DEFINE(name, type, init_val)                        \
    static type  BTF_UTILS_CONCAT(_btf_blackboard_data_, name) = init_val; \
    typedef type BTF_UTILS_CONCAT(_btf_blackboard_type_, name);            \
    struct btf_blackboard BTF_UTILS_CONCAT(_btf_blackboard_obj_, name) = { \
        .data      = &BTF_UTILS_CONCAT(_btf_blackboard_data_, name),       \
        .data_size = sizeof(type)}

#define BTF_BLACKBOARD_DECLARE(name, type)                                     \
    extern struct btf_blackboard BTF_UTILS_CONCAT(_btf_blackboard_obj_, name); \
    typedef type                 BTF_UTILS_CONCAT(_btf_blackboard_type_, name)

#define BTF_BLACKBOARD_RETRIEVE_DATA(name, member, data)                     \
    do                                                                       \
    {                                                                        \
        assert(sizeof(data)                                                  \
               == BTF_BLACKBOARD_MEMBER_SIZE(                                \
                   BTF_UTILS_CONCAT(_btf_blackboard_type_, name), member));  \
        btf_blackboard_retrieve_data(                                        \
            &BTF_UTILS_CONCAT(_btf_blackboard_obj_, name),                   \
            offsetof(BTF_UTILS_CONCAT(_btf_blackboard_type_, name), member), \
            &data, sizeof(data));                                            \
    } while (0)

#define BTF_BLACKBOARD_UPDATE_DATA(name, member, data)                       \
    do                                                                       \
    {                                                                        \
        assert(sizeof(data)                                                  \
               == BTF_BLACKBOARD_MEMBER_SIZE(                                \
                   BTF_UTILS_CONCAT(_btf_blackboard_type_, name), member));  \
        btf_blackboard_update_data(                                          \
            &BTF_UTILS_CONCAT(_btf_blackboard_obj_, name),                   \
            offsetof(BTF_UTILS_CONCAT(_btf_blackboard_type_, name), member), \
            &data, sizeof(data));                                            \
    } while (0)

#define BTF_BLACKBOARD_GET(name) BTF_UTILS_CONCAT(_btf_blackboard_obj_, name)

typedef void (*btf_blackboard_notify_cb_t)(void *context);

struct btf_blackboard
{
    // TODO: Place or calculate arch alignment
    void                      *data;
    size_t                     data_size;
    btf_blackboard_notify_cb_t notify_cb;
    void                      *notify_context;
};

int32_t btf_blackboard_retrieve_data(struct btf_blackboard *blackboard,
                                     size_t offset, void *data, size_t size);

int32_t btf_blackboard_update_data(struct btf_blackboard *blackboard,
                                   size_t offset, void *data, size_t size);

void btf_blackboard_set_notify_cb(struct btf_blackboard     *blackboard,
                                  btf_blackboard_notify_cb_t notify_cb,
                                  void                      *notify_context);

#endif  // BTF_BLACKBOARD_H
