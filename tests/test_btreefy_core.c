#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "btreefy/btreefy.h"
#include "btreefy/btreefy_objs.h"
#include "unity.h"

extern struct btf_node nodes[];
extern size_t          nodes_size;

struct test_blackboard
{
    uint16_t value;
    bool     cond1;
    bool     cond2;
};

struct node_status_mock
{
    enum btf_node_status a_cond_status;
    enum btf_node_status b_action_status;
    enum btf_node_status d_cond_status;
    enum btf_node_status e_action_status;
    enum btf_node_status g_cond_status;
    enum btf_node_status h_cond_status;
} node_status_mock;

bool e_action_aborted = false;

void setUp(void)
{
    node_status_mock.a_cond_status   = BTF_SUCCESS_STATUS;
    node_status_mock.b_action_status = BTF_SUCCESS_STATUS;
    node_status_mock.d_cond_status   = BTF_SUCCESS_STATUS;
    node_status_mock.e_action_status = BTF_SUCCESS_STATUS;
    node_status_mock.g_cond_status   = BTF_SUCCESS_STATUS;
    node_status_mock.h_cond_status   = BTF_SUCCESS_STATUS;
}

void tearDown(void)
{
    // clean stuff up here
}


enum btf_node_status d_cond(struct btf_tree *tree, enum btf_tree_signal signal)
{
    printf("D Cond executed\n");
    return node_status_mock.d_cond_status;
}

enum btf_node_status b_action(struct btf_tree     *tree,
                              enum btf_tree_signal signal)
{
    printf("B Action executed\n");
    return node_status_mock.b_action_status;
}

enum btf_node_status g_cond(struct btf_tree *tree, enum btf_tree_signal signal)
{
    printf("G Action executed\n");
    return node_status_mock.g_cond_status;
}

enum btf_node_status h_action(struct btf_tree     *tree,
                              enum btf_tree_signal signal)
{
    printf("H Action executed\n");
    return node_status_mock.h_cond_status;
}

enum btf_node_status e_action(struct btf_tree     *tree,
                              enum btf_tree_signal signal)
{
    if (signal == BTF_TICK_SIGNAL)
    {
        printf("E Action executed\n");
    }
    else if (signal == BTF_ABORT_SIGNAL)
    {
        printf("E Action was aborted\n");
        e_action_aborted = true;
    }

    return node_status_mock.e_action_status;
}

enum btf_node_status a_cond(struct btf_tree *tree, enum btf_tree_signal signal)
{
    struct test_blackboard bb;
    printf("A Cond executed\n");
    btf_tree_copy_data(tree, &bb, sizeof(bb));

    TEST_ASSERT_EQUAL_UINT16(0xCAFE, bb.value);

    return node_status_mock.a_cond_status;
}

void test_btf_init_should_initialize_tree(void)
{
    struct btf_tree        tree;
    uint32_t               tree_size  = nodes_size;
    struct test_blackboard blackboard = {
        .value = 0xCAFE, .cond1 = true, .cond2 = false};

    int32_t result = btf_init(&tree, nodes, tree_size);

    TEST_ASSERT_EQUAL_INT32(BTF_ERROR_OK, result);
    TEST_ASSERT_EQUAL_PTR(nodes, tree.nodes);
    TEST_ASSERT_EQUAL_UINT32(tree_size, tree.size);
    TEST_ASSERT_EQUAL_UINT32(BTF_NULL_NODE, (uint32_t) tree.running_node_index);

    result = btf_set_data(&tree, &blackboard, sizeof(blackboard));

    TEST_ASSERT_EQUAL_PTR(&blackboard, tree.data);
    TEST_ASSERT_EQUAL_UINT32(sizeof(blackboard), tree.datalen);

    result = btf_tick_tree(&tree);
}

void test_running_execution(void)
{
    struct btf_tree tree;
    uint32_t        tree_size = nodes_size;
    struct test_blackboard blackboard = {
        .value = 0xCAFE, .cond1 = true, .cond2 = false};

    node_status_mock.a_cond_status   = BTF_FAILURE_STATUS;
    node_status_mock.e_action_status = BTF_RUNNING_STATUS;

    int32_t result = btf_init(&tree, nodes, tree_size);

    result = btf_set_data(&tree, &blackboard, sizeof(blackboard));

    TEST_ASSERT_EQUAL_INT32(BTF_ERROR_OK, result);
    TEST_ASSERT_EQUAL_PTR(nodes, tree.nodes);
    TEST_ASSERT_EQUAL_UINT32(tree_size, tree.size);
    TEST_ASSERT_EQUAL_UINT32(BTF_NULL_NODE, (uint32_t) tree.running_node_index);

    result = btf_tick_tree(&tree);

    TEST_ASSERT_EQUAL_INT32(BTF_RUNNING_STATUS, result);
    TEST_ASSERT_EQUAL_UINT32(8, (uint32_t) tree.running_node_index);
    TEST_ASSERT_EQUAL_INT(BTF_RUNNING_STATUS, tree.nodes[0].status);
    TEST_ASSERT_EQUAL_INT(BTF_RUNNING_STATUS, tree.nodes[1].status);
    TEST_ASSERT_EQUAL_INT(BTF_RUNNING_STATUS, tree.nodes[6].status);
    TEST_ASSERT_EQUAL_INT(BTF_RUNNING_STATUS, tree.nodes[8].status);
    TEST_ASSERT_EQUAL_INT(false, e_action_aborted);

    node_status_mock.a_cond_status = BTF_SUCCESS_STATUS;

    result = btf_tick_tree(&tree);

    TEST_ASSERT_EQUAL_INT32(BTF_SUCCESS_STATUS, result);
    TEST_ASSERT_EQUAL_UINT32(BTF_NULL_NODE, (uint32_t) tree.running_node_index);
    TEST_ASSERT_EQUAL_INT(BTF_SUCCESS_STATUS, tree.nodes[0].status);
    TEST_ASSERT_EQUAL_INT(BTF_SUCCESS_STATUS, tree.nodes[1].status);
    TEST_ASSERT_EQUAL_INT(BTF_UNDEF_STATUS, tree.nodes[6].status);
    TEST_ASSERT_EQUAL_INT(BTF_UNDEF_STATUS, tree.nodes[8].status);
    TEST_ASSERT_EQUAL_INT(true, e_action_aborted);

    e_action_aborted = false;

    result = btf_tick_tree(&tree);

    TEST_ASSERT_EQUAL_INT32(BTF_SUCCESS_STATUS, result);
    TEST_ASSERT_EQUAL_UINT32(BTF_NULL_NODE, (uint32_t) tree.running_node_index);
    TEST_ASSERT_EQUAL_INT(BTF_SUCCESS_STATUS, tree.nodes[0].status);
    TEST_ASSERT_EQUAL_INT(BTF_SUCCESS_STATUS, tree.nodes[1].status);
    TEST_ASSERT_EQUAL_INT(BTF_UNDEF_STATUS, tree.nodes[6].status);
    TEST_ASSERT_EQUAL_INT(BTF_UNDEF_STATUS, tree.nodes[8].status);
    TEST_ASSERT_EQUAL_INT(false, e_action_aborted);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_btf_init_should_initialize_tree);
    RUN_TEST(test_running_execution);
    return UNITY_END();
}
