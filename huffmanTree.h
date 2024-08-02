#ifndef HUFFMANTREE_H_
#define HUFFMANTREE_H_

#include <stdint.h>

#include "hashTable.h"

typedef struct t_node t_node_t;
struct t_node
{
    t_node_t *left;
    t_node_t *right;
    int cnt;
    char letter;
};

int compare_nodes(const void *a, const void *b);
t_node_t *fuse_t_node(t_node_t *left, t_node_t *right);
t_node_t *create_t_node(char let, int cnt);
ht *createHuffTable(t_node_t *root);
uint8_t printHuffTree(t_node_t *root, FILE *fptr);

#endif