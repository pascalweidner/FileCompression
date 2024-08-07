#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "hashTable.h"

ht *resetTable(ht *tableOld)
{
    ht_destroy(tableOld);

    ht *table = ht_create();
    for (int i = 0; i < 128; i++)
    {
        char *str = (char *)malloc(2 * sizeof(char));
        str[0] = (char)i;
        str[1] = '\0';

        uint16_t *val = (uint16_t *)malloc(sizeof(uint16_t));
        *val = (uint16_t)i;

        ht_set(table, str, (void *)val);
    }

    return table;
}

bool lzwEncode(char filename[])
{
    FILE *fptr = fopen(filename, "r");

    if (fptr == NULL)
    {
        perror("Error message");
        exit(-errno);
    }

    char *fileNew = (char *)malloc((strlen(filename) * sizeof(char)));
    strcpy(fileNew, filename);
    fileNew[strlen(filename) - 4] = '\0';
    strcat(fileNew, ".lzw");

    FILE *destFptr = fopen(fileNew, "wb");
    if (destFptr == NULL)
    {
        perror("Error message");
        exit(-errno);
    }

    ht *table = ht_create();
    for (int i = 0; i < 128; i++)
    {
        char *str = (char *)malloc(2 * sizeof(char));
        str[0] = (char)i;
        str[1] = '\0';

        uint16_t *val = (uint16_t *)malloc(sizeof(uint16_t));
        *val = (uint16_t)i;

        ht_set(table, str, (void *)val);
    }

    char *P = (char *)malloc(100 * sizeof(char));
    char *C = (char *)malloc(100 * sizeof(char));
    P[0] = '\0';
    C[0] = '\0';

    uint16_t count = 128;

    uint16_t buffer = 0;
    int bufbits = 0;

    if (!feof(fptr))
    {
        P[0] = (char)fgetc(fptr);
        P[1] = '\0';
    }
    else
    {
        exit(-errno);
    }

    int ch;

    while ((ch = fgetc(fptr)) != EOF)
    {
        C[0] = (char)ch;
        C[1] = '\0';

        char *cache = (char *)malloc(200 * sizeof(char));
        strcpy(cache, P);
        strcat(cache, C);
        if (ht_get(table, cache) != NULL)
        {
            strcat(P, C);
        }
        else
        {
            while (bufbits >= 8)
            {
                uint8_t byte_to_write = (buffer >> bufbits - 8) & 0XFF;
                fwrite(&byte_to_write, sizeof(uint8_t), 1, destFptr);
                bufbits -= 8;
            }

            // output the code for P
            buffer <<= 12;
            uint16_t val = *(uint16_t *)ht_get(table, P);
            val &= 0XFFF;
            buffer |= val;
            bufbits += 12;

            // add P+C to the string table
            uint16_t *valC = (uint16_t *)malloc(sizeof(uint16_t));
            *valC = count;
            ht_set(table, cache, (void *)valC);
            count++;

            // drop table, if it is full
            if (count == 4094)
            {
                printf("drop\n");
                table = resetTable(table);
                while (bufbits >= 8)
                {
                    uint8_t byte_to_write = (buffer >> bufbits - 8) & 0XFF;
                    fwrite(&byte_to_write, sizeof(uint8_t), 1, destFptr);
                    bufbits -= 8;
                }
                buffer <<= 12;
                uint16_t val = 0XFFF;
                buffer |= val;
                bufbits += 12;
            }

            // P = C
            strcpy(P, C);
        }
    }

    // output P
    while (bufbits >= 8)
    {
        uint8_t byte_to_write = (buffer >> bufbits - 8) & 0XFF;
        fwrite(&byte_to_write, sizeof(uint8_t), 1, destFptr);
        bufbits -= 8;
    }

    buffer <<= 12;
    uint16_t val = *(uint16_t *)ht_get(table, P);
    val &= 0XFFF;
    buffer |= val;
    bufbits += 12;

    while (bufbits >= 8)
    {
        uint8_t byte_to_write = (buffer >> bufbits - 8) & 0XFF;
        fwrite(&byte_to_write, sizeof(uint8_t), 1, destFptr);
        bufbits -= 8;
    }

    printf("bufbist: %d\n", bufbits);

    free(P);
    free(C);
    ht_destroy(table);
    fclose(fptr);
    fclose(destFptr);
}