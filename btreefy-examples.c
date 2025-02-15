#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// ### BTreeFy declarations ###

#define BTF_ERROR_OK 0U
#define BTF_ERROR_STACK_IS_FULL -1
#define BTF_ERROR_STACK_IS_EMPTY -2
#define BTF_ERROR_EINVAL -3
#define BTF_ERROR_EXEC -4

#define BTF_NULL_NODE 0xFFFFFFFF

/**
 * @brief Node status enumeration
 *
 */
typedef enum
{
    BTF_UNDEF_STATUS,
    BTF_SUCCESS_STATUS,
    BTF_FAILURE_STATUS,
    BTF_RUNNING_STATUS
} btf_node_status_t;

typedef enum
{
    BTF_UNDEF_EXECUTION_RESULT,
    BTF_CONTINUE_EXECUTION_RESULT,
    BTF_RETURN_EXECUTION_RESULT,
    BTF_PAUSE_EXECUTION_RESULT
} btf_node_execution_result_t;

/**
 * @brief
 *
 */
struct btf_node;

typedef struct
{
    struct btf_node *nodes;
    uint32_t size;
    struct btf_node **tree_stack;
    uint32_t tree_stack_length;
    uint32_t visited_nodes;
} btf_tree_st;

struct btf_node
{
    uint32_t parent;
    uint32_t child;
    uint32_t sibling;
    btf_node_status_t (*action)(btf_tree_st *tree, void *data, size_t datalen);
    btf_node_execution_result_t (*control)(btf_tree_st *tree,
                                           struct btf_node *child_node,
                                           btf_node_status_t *status,
                                           void *data, size_t datalen);
};

/**
 * @brief Function prototype definition of an action executed by an Action node
 * in the tree
 *
 */
typedef btf_node_status_t (*btf_action_fn_t)(btf_tree_st *tree, void *data,
                                             size_t datalen);

/**
 * @brief Function prototype definition for a policy executed by a Control node
 * in the tree
 *
 */
typedef btf_node_execution_result_t (*btf_control_fn_t)(
    btf_tree_st *tree, struct btf_node *child_node, btf_node_status_t *status,
    void *data, size_t datalen);

// ### BTreeFy definitions ###

bool btf_is_tree_stack_empty(btf_tree_st *tree)
{
    return (tree == NULL) ? false : (tree->tree_stack_length == 0);
}

int32_t btf_push_to_tree_stack(btf_tree_st *tree, struct btf_node *node)
{
    int32_t ret = BTF_ERROR_OK;

    if ((tree == NULL) || (node == NULL))
    {
        return BTF_ERROR_EINVAL;
    }

    if (tree->tree_stack_length < tree->size)
    {
        tree->tree_stack[tree->tree_stack_length] = node;
        tree->tree_stack_length++;
    }
    else
    {
        ret = BTF_ERROR_STACK_IS_FULL;
    }

    return ret;
}

int32_t btf_pop_from_tree_stack(btf_tree_st *tree, struct btf_node *node)
{
    int32_t ret = BTF_ERROR_OK;
    uint32_t l;

    if ((tree == NULL) || (node == NULL))
    {
        return BTF_ERROR_EINVAL;
    }

    if (btf_is_tree_stack_empty(tree))
    {
        ret = BTF_ERROR_STACK_IS_EMPTY;
    }
    else
    {
        l = tree->tree_stack_length;
        memcpy(node, tree->tree_stack[l], sizeof(struct btf_node));
        tree->tree_stack_length--;
    }

    return ret;
}

btf_node_execution_result_t btf_sequence_policy_fn(btf_tree_st *tree,
                                                   struct btf_node *child_node,
                                                   btf_node_status_t *status,
                                                   void *data, size_t datalen)
{
    if (status == NULL)
    {
        printf("Error on sequence policy execution: status is NULL\n");
        return BTF_RETURN_EXECUTION_RESULT;
    }

    printf("Sequence executed!\n");

    if (BTF_SUCCESS_STATUS == *status)
    {
        return BTF_CONTINUE_EXECUTION_RESULT;
    }
    else if (BTF_FAILURE_STATUS == *status)
    {
        return BTF_RETURN_EXECUTION_RESULT;
    }
    else
    {
        // Default
    }

    return BTF_RETURN_EXECUTION_RESULT;
}

btf_node_execution_result_t btf_fallback_policy_fn(btf_tree_st *tree,
                                                   struct btf_node *child_node,
                                                   btf_node_status_t *status,
                                                   void *data, size_t datalen)
{
    if (status == NULL)
    {
        printf("Error on fallback policy execution: status is NULL\n");
        return BTF_RETURN_EXECUTION_RESULT;
    }

    printf("Fallback executed!\n");

    if (BTF_SUCCESS_STATUS == *status)
    {
        return BTF_RETURN_EXECUTION_RESULT;
    }
    else if (BTF_FAILURE_STATUS == *status)
    {
        return BTF_CONTINUE_EXECUTION_RESULT;
    }
    else
    {
        // Default
    }

    return BTF_RETURN_EXECUTION_RESULT;
}

btf_node_execution_result_t btf_success_policy_fn(btf_tree_st *tree,
                                                  struct btf_node *child_node,
                                                  btf_node_status_t *status,
                                                  void *data, size_t datalen)
{
    if (status == NULL)
    {
        printf("Error on success policy execution: status is NULL\n");
        return BTF_RETURN_EXECUTION_RESULT;
    }

    printf("Success executed!\n");

    *status = BTF_SUCCESS_STATUS;

    return BTF_RETURN_EXECUTION_RESULT;
}

btf_node_execution_result_t btf_fail_policy_fn(btf_tree_st *tree,
                                                  struct btf_node *child_node,
                                                  btf_node_status_t *status,
                                                  void *data, size_t datalen)
{
    if (status == NULL)
    {
        printf("Error on success policy execution: status is NULL\n");
        return BTF_RETURN_EXECUTION_RESULT;
    }

    printf("Fail executed!\n");

    *status = BTF_FAILURE_STATUS;

    return BTF_RETURN_EXECUTION_RESULT;
}

int32_t btf_tree_controller(btf_tree_st *tree)
{
    uint32_t node_index                  = 0;
    struct btf_node *p_node              = NULL;
    struct btf_node *p_parent            = NULL;
    struct btf_node *p_sibling           = NULL;
    struct btf_node *root                = NULL;
    btf_node_status_t status             = BTF_UNDEF_STATUS;
    btf_node_execution_result_t exec_res = BTF_UNDEF_EXECUTION_RESULT;

    if (tree == NULL)
    {
        return BTF_ERROR_EINVAL;
    }

    root = &tree->nodes[0];

    p_node   = root;
    p_parent = &tree->nodes[p_node->parent];

    // Push intermediate nodes to stack
    while (!((status != BTF_UNDEF_STATUS) && (p_node->parent == BTF_NULL_NODE)))
    {
        if (BTF_UNDEF_STATUS == status)
        {
            // Leaf node has been reached
            if (p_node->child == BTF_NULL_NODE)
            {
                status = p_node->action(tree, NULL, 0);
            }
            else
            {
                p_node   = &tree->nodes[p_node->child];
                p_parent = &tree->nodes[p_node->parent];
            }
        }
        else
        {
            // Check policy function from parent node
            exec_res = p_parent->control(tree, p_node, &status, NULL, 0);

            if (BTF_CONTINUE_EXECUTION_RESULT == exec_res)
            {
                if (p_node->sibling != BTF_NULL_NODE)
                {
                    p_node   = &tree->nodes[p_node->sibling];
                    p_parent = &tree->nodes[p_node->parent];
                    status   = BTF_UNDEF_STATUS;
                }
                else
                {
                    p_node   = &tree->nodes[p_node->parent];
                    p_parent = &tree->nodes[p_node->parent];
                }
            }
            else if (BTF_RETURN_EXECUTION_RESULT == exec_res)
            {
                p_node   = &tree->nodes[p_node->parent];
                p_parent = &tree->nodes[p_node->parent];
            }
            else
            {
                // BTF_UNDEF_EXECUTION_RESULT ||
                // BTF_PAUSE_EXECUTION_RESULT
            }
        }
    }

    return 0;
}

int32_t btf_init(btf_tree_st *tree, struct btf_node *nodes, uint32_t tree_size,
                 struct btf_node *tree_stack, uint32_t tree_stack_length)

{
    if ((tree == NULL) || (nodes == NULL) || (tree_size == 0))
    {
        return BTF_ERROR_EINVAL;
    }

    // if (tree_size != tree_stack_length)
    // {
    //     return BTF_ERROR_EINVAL;
    // }

    tree->nodes             = nodes;
    tree->size              = tree_size;

    printf("BT initialized!\n");

    return 0;
}

// -----------------------------------------------------------------------------

// ### App declarations ###

// ### App definitions ###

btf_node_status_t reset_variables_action(btf_tree_st *tree, void *data,
                                         size_t datalen)
{
    printf("Reset variables\n");
    return BTF_FAILURE_STATUS;
}

btf_node_status_t run_ble_action(btf_tree_st *tree, void *data, size_t datalen)
{
    printf("Run BLE\n");
    return BTF_FAILURE_STATUS;
}

btf_node_status_t run_lora_action(btf_tree_st *tree, void *data, size_t datalen)
{
    printf("Run LoRa\n");
    return BTF_FAILURE_STATUS;
}

btf_node_status_t go_to_sleep_action(btf_tree_st *tree, void *data,
                                     size_t datalen)
{
    printf("Go to sleep\n");
    return BTF_SUCCESS_STATUS;
}

struct btf_node nodes[] = {
    [0] = {.parent  = BTF_NULL_NODE,
           .child   = 1,
           .sibling = BTF_NULL_NODE,
           .action  = NULL,
           .control = btf_sequence_policy_fn},
    [1] = {.parent  = 0,
           .child   = 2,
           .sibling = 3,
           .action  = NULL,
           .control = btf_success_policy_fn},
    [2] = {.parent  = 1,
           .child   = BTF_NULL_NODE,
           .sibling = BTF_NULL_NODE,
           .action  = reset_variables_action,
           .control = NULL},
    [3] = {.parent  = 0,
           .child   = 4,
           .sibling = 6,
           .action  = NULL,
           .control = btf_fallback_policy_fn},
    [4] = {.parent  = 3,
           .child   = BTF_NULL_NODE,
           .sibling = 5,
           .action  = run_ble_action,
           .control = NULL},
    [5] = {.parent  = 3,
           .child   = BTF_NULL_NODE,
           .sibling = BTF_NULL_NODE,
           .action  = run_lora_action,
           .control = NULL},
    [6] = {.parent  = 0,
           .child   = 7,
           .sibling = BTF_NULL_NODE,
           .action  = NULL,
           .control = btf_success_policy_fn},
    [7] = {.parent  = 6,
           .child   = BTF_NULL_NODE,
           .sibling = BTF_NULL_NODE,
           .action  = go_to_sleep_action,
           .control = NULL},
};

int main(void)
{
    btf_tree_st tree;

    if (btf_init(&tree, nodes, sizeof(nodes), NULL, 0) == 0)
    {
        btf_tree_controller(&tree);
    }
    return 0;
}