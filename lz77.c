#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>

#define BLOCK_SIZE 32000

FILE *openDestFile(char filename[], char end[], char mode[])
{
    char *fileNew = (char *)malloc((strlen(filename) + 1) * sizeof(char));
    strcpy(fileNew, filename);
    fileNew[strlen(filename) - 4] = '\0';
    strcat(fileNew, end);

    FILE *destFptr = fopen(fileNew, mode);
    if (destFptr == NULL)
    {
        perror("Error message");
        exit(-errno);
    }

    free(fileNew);

    return destFptr;
}

void readBlock(FILE *fptr, char *buffer)
{
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        if (!feof(fptr))
        {
            fread(buffer + i, sizeof(uint8_t), 1, fptr);
        }
    }
}

void encodeBlock(FILE *fptr, FILE *destFptr, char *input)
{
}

// <flag bit><offset bit><7 | 15 offset value bit><length bit><7 | 15 length value bit><7 assci bit>
bool encodeLZ77(char filename[])
{
    FILE *fptr = fopen(filename, "rb");

    if (fptr == NULL)
    {
        perror("Error message");
        exit(-errno);
    }

    FILE *destFptr = openDestFile(filename, "lz77", "wb");

    int curren_position = 0;

    char *buffer = (char *)malloc(BLOCK_SIZE * sizeof(char));

    while (!feof(fptr))
    {
        readBlock(fptr, buffer);
    }

    encodeBlock(fptr, destFptr, buffer);
}