#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// ### BTreeFy declarations ###

#define BTF_ERROR_OK              0U
#define BTF_ERROR_STACK_IS_FULL   -1
#define BTF_ERROR_STACK_IS_EMPTY  -2
#define BTF_ERROR_EINVAL          -3
#define BTF_ERROR_EXEC            -4

/**
 * @brief Node status enumeration
 * 
 */
typedef enum {
    BTF_UNDEF_STATUS,
    BTF_SUCCESS_STATUS,
    BTF_FAILURE_STATUS,
    BTF_RUNNING_STATUS
} btf_node_status_t;

/**
 * @brief 
 * 
 */
struct btf_node;

struct btf_node
{
    uint32_t child;
    uint32_t sibling;
    btf_action_fn_t action;
    bool is_leaf;
};

typedef struct
{
    struct btf_node * nodes;    
    uint32_t size;    
    struct btf_node ** tree_stack;    
    uint32_t tree_stack_length;
    uint32_t visited_nodes;
} btf_tree_st;

/**
 * @brief Function prototype definition of an action executed by an Action node in the tree
 * 
 */
typedef btf_node_status_t (*btf_action_fn_t)(btf_tree_st * tree, void *data, size_t datalen);


// ### BTreeFy definitions ###

bool btf_is_tree_stack_empty(btf_tree_st * tree)
{
    return (tree == NULL) ? false : (tree->tree_stack_length == 0);
}

int32_t btf_push_to_tree_stack(btf_tree_st * tree, struct btf_node * node)
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

int32_t btf_pop_from_tree_stack(btf_tree_st * tree, struct btf_node * node)
{
    int32_t ret  = BTF_ERROR_OK;
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

btf_node_status_t btf_sequence_node_fn(void *data, size_t datalen)
{
    return BTF_SUCCESS_STATUS;
}

int32_t btf_tree_controller(btf_tree_st * tree)
{
    uint32_t node_index = 0;
    struct btf_node * p_node = NULL;
    struct btf_node * p_ret_node = NULL;
    btf_node_status_t previous_status = BTF_UNDEF_STATUS;

    if (tree == NULL)
    {
        return BTF_ERROR_EINVAL;
    }

    while (tree->visited_nodes < tree->size)
    {
        node_index = tree->tree_stack_length;
        p_node = &tree->nodes[node_index];

        if (tree->visited_nodes == tree->tree_stack_length)
        {
            if (btf_push_to_tree_stack(tree, p_node))
            {
                return BTF_ERROR_EXEC;
            }
            tree->visited_nodes++;

            if (p_node->is_leaf)
            {
                previous_status = p_node->action(tree, NULL, 0);
                if (btf_pop_from_tree_stack(tree, p_ret_node))
                {
                    return BTF_ERROR_EXEC;
                }
            }
        }
        else
        {
        }
    }
}

int32_t btf_init(btf_tree_st * tree, struct btf_node * nodes, 
                 uint32_t tree_size, struct btf_node * tree_stack,
                uint32_t tree_stack_length)
                
{
    if ((tree == NULL) || (nodes == NULL) || (tree_size == 0))
    {
        return BTF_ERROR_EINVAL;
    }

    if (tree_size != tree_stack_length)
    {
        return BTF_ERROR_EINVAL;
    }

    tree->nodes = nodes;
    tree->size = tree_size;
    tree->tree_stack = &tree_stack;
    tree->tree_stack_length = tree_stack_length;
    tree->visited_nodes = 0;

    return 0;
}

// -----------------------------------------------------------------------------

// ### App declarations ###

void set_operation_mode(uint8_t value);

// ### App definitions ###

uint8_t operation_mode = 0;

btf_node_status_t peripherals_setup(void *data, size_t datalen)
{
    return BTF_SUCCESS_STATUS;
}

btf_node_status_t get_operation_mode(void *data, size_t datalen)
{
    btf_node_status_t ret = BTF_FAILURE_STATUS;
    
    if (operation_mode > 0)
    {
        ret = BTF_SUCCESS_STATUS;
    }

    return ret;
}

// ### Helper functions definitions


void set_operation_mode(uint8_t value)
{
    operation_mode = value;
}

int main(void)
{
    return 0;
}