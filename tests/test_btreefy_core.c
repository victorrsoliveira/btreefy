#include <stddef.h>
#include <stdint.h>

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

static uint32_t call_mask = 0;

void setUp(void)
{
    call_mask = 0;
}

void tearDown(void)
{
    // clean stuff up here
}

enum btf_node_status d_cond(struct btf_tree *tree)
{
    call_mask |= (1 << 2);
    return BTF_SUCCESS_STATUS;
}

enum btf_node_status b_action(struct btf_tree *tree)
{
    call_mask |= (1 << 1);
    return BTF_SUCCESS_STATUS;
}

enum btf_node_status g_cond(struct btf_tree *tree)
{
    call_mask |= (1 << 4);
    return BTF_SUCCESS_STATUS;
}

enum btf_node_status h_action(struct btf_tree *tree)
{
    call_mask |= (1 << 5);
    return BTF_SUCCESS_STATUS;
}

enum btf_node_status e_action(struct btf_tree *tree)
{
    call_mask |= (1 << 3);
    return BTF_SUCCESS_STATUS;
}

enum btf_node_status a_cond(struct btf_tree *tree)
{
    call_mask |= (1 << 0);
    struct test_blackboard bb;
    btf_tree_copy_data(tree, &bb, sizeof(bb));

    TEST_ASSERT_EQUAL_UINT16(0xCAFE, bb.value);

    return BTF_SUCCESS_STATUS;
}

void test_btf_init_should_initialize_tree(void)
{
    struct btf_tree            tree;
    uint32_t               tree_size  = nodes_size;
    struct test_blackboard blackboard = {.value = 0xCAFE,
                                         .cond1 = true,
                                         .cond2 = false};

    int32_t result = btf_init(&tree, nodes, tree_size);

    TEST_ASSERT_EQUAL_INT32(BTF_ERROR_OK, result);
    TEST_ASSERT_EQUAL_PTR(nodes, tree.nodes);
    TEST_ASSERT_EQUAL_UINT32(tree_size, tree.size);
    TEST_ASSERT_EQUAL_UINT32(BTF_NULL_NODE, (uint32_t) tree.running_node_index);

    result = btf_set_data(&tree, &blackboard, sizeof(blackboard));

    TEST_ASSERT_EQUAL_PTR(&blackboard, tree.data);
    TEST_ASSERT_EQUAL_UINT32(sizeof(blackboard), tree.datalen);

    result = btf_tick_tree(&tree);
    TEST_ASSERT_EQUAL_INT32(BTF_SUCCESS_STATUS, result);

    // Verify traversal path (only A, B, and G should be invoked):
    // a_cond (1 << 0) = 1
    // b_action (1 << 1) = 2
    // g_cond (1 << 4) = 16
    // Expected call mask = 1 + 2 + 16 = 19
    TEST_ASSERT_EQUAL_UINT32((1 << 0) | (1 << 1) | (1 << 4), call_mask);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_btf_init_should_initialize_tree);
    return UNITY_END();
}
