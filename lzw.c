#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "hashTable.h"

bool lzwEncode(char filename[])
{
    FILE *fptr = fopen(filename, "r");

    if (fptr == NULL)
    {
        perror("Error message");
        exit(-errno);
    }

    ht *table = ht_create();

    char *P = (char *)malloc(100 * sizeof(char));
    char *C = (char *)malloc(100 * sizeof(char));

    uint16_t buffer = 0;

    if (!feof(fptr))
    {
        strcpy(P, fgetc(fptr));
    }
    else
    {
        exit(-errno);
    }
    while (!feof(fptr))
    {
        strcpy(C, fgetc(fptr));
        char cache[200];
        strcpy(cache, P);
        strcat(cache, C);
        if (ht_get(table, cache) != NULL)
        {
            strcat(P, C);
        }
        else
        {
            buffer <<= 12;
        }
    }
}