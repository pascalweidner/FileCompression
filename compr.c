#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

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

    printf("Test1\n");

    FILE *fptr;
    fptr = fopen(filepath, "r");
    if (fptr == NULL)
    {
        perror("Error message");
        exit(-errno);
    }

    ht *table = ht_create();
    printf("%p \n", table);

    char buffer[100];
    while (fgets(buffer, 100, fptr))
    {
        for (int i = 0; i < 100; i++)
        {
            char key[2] = {buffer[i], '\0'};
            int val = ht_get(table, key);
            if (val == -1)
            {
                if (ht_set(table, key, 1) == NULL)
                {
                    perror("Error message");
                    exit(-errno);
                }
            }
            else
            {
                if (ht_set(table, key, val + 1) == NULL)
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
    t_node_t **array = malloc(ht_length(table) * sizeof(t_node_t *));

    hti it = ht_iterator(table);

    int i = 0;

    while (ht_next(&it) != false)
    {
        t_node_t *new = create_t_node(it.key[0], it.value);
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

bool huffmanEncode(char filepath[])
{
    ht *table = countCharacters(filepath);
    printf("count: %d\n", ht_length(table));

    h_list *huffList = createHuffList(table);

    t_node_t *root = buildHuffmanTree(huffList);

    ht_destroy(table);
}

int main()
{
    huffmanEncode("/mnt/d/Development/Languages/C/FileCompression/files/test.txt");

    return 0;
}