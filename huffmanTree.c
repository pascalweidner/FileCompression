#include <stdio.h>
#include <stdlib.h>

typedef struct t_node t_node_t;

struct t_node
{
    t_node_t *left;
    t_node_t *right;
    int cnt;
    char letter;
};

t_node_t *create_node(t_node_t *left, t_node_t *right)
{
    t_node_t *new_node = (t_node_t *)malloc(sizeof(t_node_t));
    new_node->left = left;
    new_node->right = right;
    int cnt = left->cnt + right->cnt;
    new_node->letter = NULL;
}
