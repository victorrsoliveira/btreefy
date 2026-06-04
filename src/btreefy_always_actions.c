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


enum btf_node_status btf_always_success_action(struct btf_tree *tree, enum btf_tree_signal signal)
{
    return BTF_SUCCESS_STATUS;
}

enum btf_node_status btf_always_failure_action(struct btf_tree *tree, enum btf_tree_signal signal)
{
    return BTF_FAILURE_STATUS;
}