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
        printf("buffer: %d\n", *buffer);
        printf("bufbits: %d\n", *bufbits);
        printf("byte_to_write %d\n", byte_to_write);

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

t_node_t *travRead(FILE *fptr, uint16_t *buffer, int *bufbits)
{
    printf("buffer: %d\n", *buffer);
    printf("bufbits: %d\n", *bufbits);
    // if buffer is empty fill it up with a new byte from the file
    if (*bufbits == 8)
    {
        uint8_t cache;
        fread(&cache, sizeof(uint8_t), 1, fptr);
        *buffer |= cache;
        *bufbits += 8;
    }

    // get last bit of byte
    // printf("buffer: %d\n", *buffer);
    uint8_t bit_to_read = (*buffer) >> 15;
    printf("bit_to_read %d\n", bit_to_read);
    // printf("bit_to_read: %d\n", bit_to_read);
    if (bit_to_read == 1)
    {
        printf("buffer2: %d\n", (*buffer >> 7) & 0XFF);
        printf("test: %c\n", 163);
        unsigned char let = (uint8_t)((*buffer >> 7) & 0XFF);
        printf("let: %c \n", let);
        *buffer <<= (*bufbits - 8);

        uint8_t cache;
        fread(&cache, sizeof(uint8_t), 1, fptr);
        *buffer |= cache;

        *buffer <<= (8 - (*bufbits - 9));

        // remove the one node bit from the count
        *bufbits -= 1;
        return create_t_node(let, 0);
    }

    *buffer <<= 1;
    *bufbits -= 1;
    t_node_t *leftChild = travRead(fptr, buffer, bufbits);
    t_node_t *rightChild = travRead(fptr, buffer, bufbits);
    return fuse_t_node(leftChild, rightChild);
}

t_node_t *readHuffTree(FILE *fptr)
{
    uint16_t buffer = 0;
    uint8_t cache;
    fread(&cache, sizeof(uint8_t), 1, fptr);
    buffer |= cache;
    buffer <<= 8;
    fread(&cache, sizeof(uint8_t), 1, fptr);
    buffer |= cache;

    int bufbits = 16;

    return travRead(fptr, &buffer, &bufbits);
}
