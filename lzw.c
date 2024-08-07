#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "hashTable.h"

ht *buildTable()
{
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

ht *resetTable(ht *tableOld)
{
    ht_destroy(tableOld);

    return buildTable();
}

void writeBufferToFile(FILE *fptr, uint16_t *buffer, int *bufbits)
{
    while (*bufbits >= 8)
    {
        uint8_t byte_to_write = (*buffer >> *bufbits - 8) & 0XFF;
        fwrite(&byte_to_write, sizeof(uint8_t), 1, fptr);
        *bufbits -= 8;
    }
}

// adds the 12 bit value in val to the buffer, so it can be written to the file as bytes
void outputVal(uint16_t val, uint16_t *buffer, int *bufbits)
{
    *buffer <<= 12;
    val &= 0XFFF;
    *buffer |= val;
    *bufbits += 12;
}

void output(FILE *fptr, uint16_t val, uint16_t *buffer, int *bufbits)
{
    writeBufferToFile(fptr, buffer, bufbits);
    outputVal(val, buffer, bufbits);
}

void PplusC(char *cache, char *P, char *C)
{
    strcpy(cache, P);
    strcat(cache, C);
}

FILE *openDestFile(char filename[])
{
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

    return destFptr;
}

bool lzwEncode(char filename[])
{
    FILE *fptr = fopen(filename, "r");

    if (fptr == NULL)
    {
        perror("Error message");
        exit(-errno);
    }

    FILE *destFptr = openDestFile(filename);

    // initializes the table with all the ascii characters, so 0 - 127
    ht *table = buildTable();

    // declares P and C as strings
    char *P = (char *)malloc(100 * sizeof(char));
    char *C = (char *)malloc(100 * sizeof(char));
    P[0] = '\0';
    C[0] = '\0';

    uint16_t codeCount = 128;
    uint16_t buffer = 0;
    // holds the value, of how many bits are currently in the buffer and not written to file
    int bufbits = 0;
    int ch;
    char *cache = (char *)malloc(200 * sizeof(char));

    // gets the first character
    if (!feof(fptr))
    {
        P[0] = (char)fgetc(fptr);
        P[1] = '\0';
    }
    else
    {
        return true;
    }

    while ((ch = fgetc(fptr)) != EOF)
    {
        C[0] = (char)ch;
        C[1] = '\0';

        PplusC(cache, P, C);
        if (ht_get(table, cache) != NULL)
        {
            strcat(P, C);
        }
        else
        {
            // writes the buffer to the file, and adds the 12 bit value to the buffer
            output(destFptr, *(uint16_t *)ht_get(table, P), &buffer, &bufbits);

            // add P+C to the string table
            uint16_t *valC = (uint16_t *)malloc(sizeof(uint16_t));
            *valC = codeCount;
            ht_set(table, cache, (void *)valC);
            codeCount++;

            // drop table, if it is full and start a new one
            // it adds a certain code to the file, so that the decompression can do so accordingly
            if (codeCount == 4094)
            {
                table = resetTable(table);
                writeBufferToFile(destFptr, &buffer, &bufbits);
                outputVal(0XFFF, &buffer, &bufbits);
            }

            // P = C
            strcpy(P, C);
        }
    }

    // output P
    output(destFptr, *(uint16_t *)ht_get(table, P), &buffer, &bufbits);

    // clear buffer
    writeBufferToFile(destFptr, &buffer, &bufbits);

    printf("bufbist: %d\n", bufbits);

    free(P);
    free(C);
    free(cache);
    ht_destroy(table);
    fclose(fptr);
    fclose(destFptr);
}