#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>

#include "hashTable.h"
#include "huffmanTree.h"
#include "list_h.h"

bool checkIfTxt(char *filepath)
{
    char *ending = strrchr(filepath, '.');
    return strcmp(ending, ".txt") == 0;
}

ht *countCharacters(char *filepath)
{
    if (checkIfTxt(filepath) == false)
    {
        return NULL;
    }

    FILE *fptr;
    fptr = fopen(filepath, "r");
    if (fptr == NULL)
    {
        perror("Error message");
        exit(-errno);
    }

    ht *table = ht_create();

    char buffer[100];
    while (fgets(buffer, 100, fptr))
    {
        for (int i = 0; i < 100; i++)
        {
            char *key = (char *)malloc(2 * sizeof(char));
            key[0] = buffer[i];
            key[1] = '\0';
            int *val = (int *)ht_get(table, key);
            if (val == NULL)
            {
                int *new = (int *)malloc(sizeof(int));
                *new = 1;
                if (ht_set(table, key, (void *)new) == NULL)
                {
                    perror("Error message");
                    exit(-errno);
                }
            }
            else
            {
                *val += 1;
                if (ht_set(table, key, (void *)(val)) == NULL)
                {
                    perror("Error message");
                    exit(-errno);
                }
            }
        }
    }

    fclose(fptr);

    return table;
}

h_list *createHuffList(ht *table)
{
    t_node_t **array = (t_node_t **)malloc(ht_length(table) * sizeof(t_node_t *));

    hti it = ht_iterator(table);

    int i = 0;

    while (ht_next(&it) != false)
    {
        t_node_t *new = create_t_node(it.key[0], *(int *)it.value);
        array[i] = new;
        i++;
    }

    qsort(array, ht_length(table), sizeof(t_node_t *), compare_nodes);

    h_list *list = createHList();
    for (int i = 0; i < ht_length(table); i++)
    {
        t_node_t *n = array[i];
        add_h(list, n);
    }

    return list;
}

t_node_t *buildHuffmanTree(h_list *lst)
{
    while (list_len(lst) > 1)
    {
        t_node_t *node1 = pop(lst);
        t_node_t *node2 = pop(lst);
        t_node_t *newNode = fuse_t_node(node1, node2);

        if (list_len(lst) == 0)
        {
            add_h(lst, newNode);
            break;
        }

        int i = 0;
        t_node_t *curr = get_h(lst, i);
        while (curr->cnt < newNode->cnt && list_len(lst) > i)
        {
            curr = get_h(lst, i);
            i++;
        }
        insert_h(lst, i, newNode);
    }

    return get_h(lst, 0);
}

void compressFile(ht *encTable, char filepath[], t_node_t *root)
{
    char *end = strrchr(filepath, '.');
    char *fileNew = (char *)malloc((strlen(filepath) + 2) * sizeof(char));
    strcpy(fileNew, filepath);
    fileNew[strlen(filepath) - 4] = '\0';
    strcat(fileNew, "HE.txt");

    FILE *fptr1;
    fptr1 = fopen(filepath, "r");
    if (fptr1 == NULL)
    {
        perror("Error message");
        exit(-errno);
    }

    FILE *fptr2;
    fptr2 = fopen(fileNew, "wb");

    // write first byte only zeros to make a placehol for the padding
    uint8_t puffer = 0b00000000;
    fwrite(&puffer, sizeof(uint8_t), 1, fptr2);

    // write the huffman Tree
    uint8_t paddingTree = printHuffTree(root, fptr2);

    // encode the file data with the huffmann Table
    char buffer[100];
    uint8_t buffer2 = 0;
    int bufbits = 0;
    while (fgets(buffer, 100, fptr1))
    {
        for (int i = 0; i < 100; i++)
        {
            char key[2] = {buffer[i], '\0'};
            char *code = (char *)ht_get(encTable, key);
            for (int j = 0; j < 20; j++)
            {
                if (code[j] == '\0')
                {
                    break;
                }
                buffer2 <<= 1;
                bufbits += 1;
                buffer2 |= (code[j] == '1' ? 1 : 0);

                if (bufbits == 8)
                {
                    bufbits -= 8;
                    fwrite(&buffer2, 1, 1, fptr2);
                    buffer2 = 0;
                }
            }
        }
    }

    // fill the buffer to a full byte, so everything is saved
    if (bufbits != 0)
    {
        buffer2 <<= (8 - bufbits);
        fwrite(&buffer, sizeof(uint8_t), 1, fptr2);
    }

    // padding
    uint8_t paddingCompr = (uint8_t)(8 - bufbits);

    printf("len1: %d\n", paddingTree);
    printf("len2: %d \n", paddingCompr);

    // write padding to file
    uint8_t padding = 0b00000000;
    padding |= paddingTree;
    padding <<= 4;
    padding |= paddingCompr;

    fseek(fptr2, 0, SEEK_SET);
    fwrite(&padding, sizeof(uint8_t), 1, fptr2);

    fclose(fptr1);
    fclose(fptr2);
    free(fileNew);
}

bool huffmanEncode(char filepath[])
{
    ht *table = countCharacters(filepath);
    printf("count: %d\n", (int)ht_length(table));

    h_list *huffList = createHuffList(table);
    ht_destroy(table);

    t_node_t *root = buildHuffmanTree(huffList);
    destroyHList(huffList);

    ht *encTable = createHuffTable(root);
    printf("%d\n", (int)ht_length(encTable));

    compressFile(encTable, filepath, root);

    ht_destroy(encTable);
}

void decompressFile(FILE *fptr, t_node_t *root, char filepath[])
{
    char *end = strrchr(filepath, '.');
    char *fileNew = (char *)malloc((strlen(filepath) + 2) * sizeof(char));
    strcpy(fileNew, filepath);
    fileNew[strlen(filepath) - 6] = '\0';
    strcat(fileNew, "HD.txt");

    FILE *destFptr = fopen(fileNew, "w");

    printf("Test\n");

    uint8_t buffer = 0;
    int bufbits = 0;
    t_node_t *curr = root;
    printf("root: %d\n", curr->letter);

    printf("Test1\n");

    while (true)
    {
        printf("Test2\n");
        if (bufbits == 0)
        {
            if (fread(&buffer, sizeof(uint8_t), 1, fptr) == 0)
            {
                break;
            }
            bufbits = 8;
        }

        printf("Test3\n");
        uint8_t bit_to_read = buffer >> 7;
        if (bit_to_read == 1)
        {
            curr = curr->left;
        }
        else
        {
            curr = curr->right;
        }
        printf("Test4\n");

        buffer <<= 1;
        bufbits -= 1;

        // check if we reached a leaf == a letter
        if (curr->letter != -1)
        {
            char str[2] = "\0";
            str[0] = curr->letter;
            fputs(str, destFptr);
        }
    }

    free(destFptr);
    fclose(destFptr);
}

bool huffmanDecode(char filepath[])
{
    FILE *fptr = fopen(filepath, "rb");
    if (fptr == NULL)
    {
        perror("Error message");
        exit(-errno);
    }

    uint8_t padding;
    fread(&padding, sizeof(uint8_t), 1, fptr);

    // get the different paddings from the padding byte
    uint8_t paddingTree = padding >> 4;
    uint8_t paddingCompr = padding & 0xF;

    t_node_t *root = readHuffTree(fptr);
    printf("test\n");

    fseek(fptr, -1, SEEK_CUR);

    decompressFile(fptr, root, filepath);

    fclose(fptr);
}

int main()
{
    // huffmanEncode("/mnt/d/Development/Languages/C/FileCompression/files/test.txt");
    huffmanDecode("/mnt/d/Development/Languages/C/FileCompression/files/testHE.txt");

    return 0;
}