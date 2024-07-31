#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include "hashTable.h"

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
                if (ht_set(table, key, 0) == NULL)
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

bool huffmanEncode(char filepath[])
{
    ht *table = countCharacters(filepath);
    printf("%d \n", (int)ht_length(table));
    ht_destroy(table);
}

int main()
{
    huffmanEncode("/mnt/d/Development/Languages/C/FileCompression/files/test.txt");

    return 0;
}