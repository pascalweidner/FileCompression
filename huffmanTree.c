#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "huffmanTree.h"
#include "hashTable.h"

typedef struct t_node t_node_t;

t_node_t *fuse_t_node(t_node_t *left, t_node_t *right)
{
    t_node_t *new_node = (t_node_t *)malloc(sizeof(t_node_t));
    new_node->left = left;
    new_node->right = right;
    new_node->letter = -1;
    new_node->cnt = left->cnt + right->cnt;
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

void inverse(t_node_t *node, ht *table, char code[20], int i)
{
    if (node->left != NULL)
    {
        char leftCode[20];
        strcpy(leftCode, code);
        leftCode[i] = '1';
        inverse(node->left, table, leftCode, i + 1);
    }
    if (node->left == NULL && node->right == NULL)
    {
        char *key = (char *)malloc(2 * sizeof(char));
        key[0] = node->letter;
        key[1] = '\0';

        char *newCode = (char *)malloc(20 * sizeof(char));
        strcpy(newCode, code);
        newCode[i] = '\0';

        ht_set(table, key, (void *)newCode);
        return;
    }
    if (node->right != NULL)
    {
        char rightCode[20];
        strcpy(rightCode, code);
        rightCode[i] = '0';
        inverse(node->right, table, rightCode, i + 1);
    }
}

ht *iterate_tree(t_node_t *root)
{
    ht *table = ht_create();
    char code[20];
    memset(code, '\0', 20);
    inverse(root, table, code, 0);
    return table;
}
