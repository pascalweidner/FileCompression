#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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

void inorderTrav(t_node_t *node, ht *table, char code[20], int i)
{
    if (node->left != NULL)
    {
        char leftCode[20];
        strcpy(leftCode, code);
        leftCode[i] = '1';
        inorderTrav(node->left, table, leftCode, i + 1);
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
        inorderTrav(node->right, table, rightCode, i + 1);
    }
}

ht *createHuffTable(t_node_t *root)
{
    ht *table = ht_create();
    char code[20];
    memset(code, '\0', 20);
    inorderTrav(root, table, code, 0);
    return table;
}

void travPrint(t_node_t *node, FILE *fptr, uint16_t *buffer, int *bufbits)
{
    if (node->letter == -1)
    {
        *buffer <<= 1;
        *bufbits += 1;
        *buffer |= 0;
    }
    else
    {
        char let = node->letter;
        *buffer <<= 1;
        *buffer |= 1;
        *buffer <<= 8;
        *buffer |= let;
        *bufbits += 9;
    }

    while (*bufbits >= 8)
    {
        uint8_t byte_to_write = (*buffer >> (*bufbits - 8)) & 0xFF;

        fwrite(&byte_to_write, sizeof(uint8_t), 1, fptr);

        *buffer = *buffer & ((1 << (*bufbits - 8)) - 1);
        *bufbits -= 8;
    }

    if (node->left != NULL)
    {
        travPrint(node->left, fptr, buffer, bufbits);
    }
    if (node->right != NULL)
    {
        travPrint(node->right, fptr, buffer, bufbits);
    }
}

uint8_t printHuffTree(t_node_t *root, FILE *fptr)
{
    uint16_t buffer = 0;
    int bufbits = 0;

    travPrint(root, fptr, &buffer, &bufbits);
    if (bufbits != 0)
    {
        buffer <<= (8 - bufbits);
        buffer &= 0xFF;
        fwrite(&buffer, sizeof(uint8_t), 1, fptr);
    }

    return 8 - bufbits;
}
