/**
 * @file btreefy_always_actions.c
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2026-04-30
 * 
 */

#include "btreefy/btreefy_always_actions.h"
#include "btreefy/btreefy_objs.h"


btf_node_status_t btf_always_success_action(btf_tree_st *tree, void *data, size_t datalen)
{
    return BTF_SUCCESS_STATUS;
}

btf_node_status_t btf_always_failure_action(btf_tree_st *tree, void *data, size_t datalen)
{
    return BTF_FAILURE_STATUS;
}