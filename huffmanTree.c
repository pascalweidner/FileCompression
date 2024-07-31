#include <stdio.h>
#include <stdlib.h>

#include "huffmanTree.h"

typedef struct t_node t_node_t;

t_node_t *fuse_t_node(t_node_t *left, t_node_t *right)
{
    t_node_t *new_node = (t_node_t *)malloc(sizeof(t_node_t));
    new_node->left = left;
    new_node->right = right;
    int cnt = left->cnt + right->cnt;
    new_node->letter = -1;
}

int compare_nodes(const void *a, const void *b)
{
    int int_a = (*((t_node_t **)a))->cnt;
    int int_b = (*((t_node_t **)b))->cnt;

    if (int_a == int_b)
        return 0;
    else if (int_a < int_b)
        return -1;
    else
        return 1;
}

t_node_t *create_t_node(char let, int cnt)
{
    t_node_t *new_node = (t_node_t *)malloc(sizeof(t_node_t));
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->cnt = cnt;
    new_node->letter = let;
    return new_node;
}
