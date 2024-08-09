#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "hashTable.h"

ht *buildEncTable()
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

ht *buildDecTable()
{
    ht *table = ht_create();
    for (int i = 0; i < 128; i++)
    {
        uint16_t *key = (uint16_t *)malloc(sizeof(uint16_t));
        *key = (uint16_t)i;

        char *val = (char *)malloc(2 * sizeof(char));
        val[0] = (char)i;
        val[1] = '\0';

        ht_set_gen(table, (void *)key, sizeof(uint16_t), (void *)val);
    }

    return table;
}

ht *resetEncTable(ht *tableOld)
{
    ht_destroy(tableOld);

    return buildEncTable();
}

ht *resetDecTable(ht *tableOld)
{
    ht_destroy(tableOld);

    return buildDecTable();
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

FILE *openDestFile(char filename[], char end[], char mode[])
{
    char *fileNew = (char *)malloc((strlen(filename) * sizeof(char)));
    strcpy(fileNew, filename);
    fileNew[strlen(filename) - 4] = '\0';
    strcat(fileNew, end);

    FILE *destFptr = fopen(fileNew, mode);
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

    FILE *destFptr = openDestFile(filename, ".lzw", "wb");

    // initializes the table with all the ascii characters, so 0 - 127
    ht *table = buildEncTable();

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

        char *cache = (char *)malloc(200 * sizeof(char));
        PplusC(cache, P, C);
        if (ht_get(table, cache) != NULL)
        {
            strcat(P, C);
        }
        else
        {
            // writes the buffer to the file, and adds the 12 bit value to the buffer
            output(destFptr, *(uint16_t *)ht_get(table, P), &buffer, &bufbits);

            // drop table, if it is full and start a new one
            // it adds a certain code to the file, so that the decompression can do so accordingly
            if (codeCount == 4095)
            {
                table = resetEncTable(table);
                codeCount = 128;
                printf("drop\n");
                writeBufferToFile(destFptr, &buffer, &bufbits);
                outputVal(0XFFF, &buffer, &bufbits);
            }

            // add P+C to the string table
            uint16_t *valC = (uint16_t *)malloc(sizeof(uint16_t));
            *valC = codeCount;
            ht_set(table, cache, (void *)valC);
            codeCount++;

            // P = C
            strcpy(P, C);
        }
    }

    // output P
    output(destFptr, *(uint16_t *)ht_get(table, P), &buffer, &bufbits);

    // clear buffer
    writeBufferToFile(destFptr, &buffer, &bufbits);

    if (bufbits != 0)
    {
        uint8_t padding = 8 - bufbits;
        // add padding bits
        buffer <<= padding;
        bufbits += padding;
        printf("buffer: %d\n", buffer);
        writeBufferToFile(destFptr, &buffer, &bufbits);
    }

    free(P);
    free(C);
    ht_destroy(table);
    fclose(fptr);
    fclose(destFptr);

    return true;
}

void readFromBuffer(uint16_t *buffer, int *bufbits, uint16_t *dest)
{
    if (*bufbits < 12)
    {
        return;
    }

    *dest = (*buffer >> (*bufbits - 12)) & 0XFFF;
    *bufbits -= 12;
}

void readToBuffer(FILE *fptr, uint16_t *buffer, int *bufbits)
{
    while (*bufbits <= 8)
    {
        uint8_t cache = 0;
        fread(&cache, sizeof(uint8_t), 1, fptr);
        if (feof(fptr))
        {
            break;
        }
        *buffer <<= 8;
        *buffer |= cache;
        *bufbits += 8;
    }
    printf("bufbits: %d\n", *bufbits);

    if (feof(fptr))
    {
        printf("buffer: %d\n", *buffer);
        printf("buf: %d\n", *bufbits);
    }
}

void input(FILE *fptr, uint16_t *buffer, int *bufbits, uint16_t *dest)
{
    readToBuffer(fptr, buffer, bufbits);
    readFromBuffer(buffer, bufbits, dest);
}

bool lzwDecode(char filename[])
{
    FILE *fptr = fopen(filename, "rb");

    if (fptr == NULL)
    {
        perror("Error message");
        exit(-errno);
    }

    FILE *destFptr = openDestFile(filename, ".txt", "w");

    uint8_t byte;
    fseek(fptr, -1, SEEK_END);
    printf("end: %d\n", feof(fptr));
    fread(&byte, sizeof(uint8_t), 1, fptr);
    printf("byte: %d\n", byte);
    fseek(fptr, 0, SEEK_SET);

    // initialize table with single character strings
    ht *table = buildDecTable();

    uint16_t OLD;
    uint16_t NEW;

    char OLDcache[100];

    char *S = (char *)malloc(100 * sizeof(char));
    char *C = (char *)malloc(100 * sizeof(char));
    S[0] = '\0';
    C[0] = '\0';

    uint16_t codeCount = 128;
    uint16_t buffer = 0;
    int bufbits = 0;

    if (!feof(fptr))
    {
        input(fptr, &buffer, &bufbits, &OLD);
    }
    else
    {
        return true;
    }

    fputs((char *)ht_get_gen(table, &OLD, sizeof(uint16_t)), destFptr);

    while (!feof(fptr))
    {
        input(fptr, &buffer, &bufbits, &NEW);
        if (feof(fptr))
        {
            break;
        }
        if (NEW == 4095)
        {
            strcpy(OLDcache, (char *)ht_get_gen(table, &OLD, sizeof(uint16_t)));
            table = resetDecTable(table);
            codeCount = 128;
            continue;
        }

        if (ht_get_gen(table, &NEW, sizeof(uint16_t)) == NULL)
        {
            S = (char *)ht_get_gen(table, &OLD, sizeof(uint16_t));
            strcat(S, C);
        }
        else
        {
            S = (char *)ht_get_gen(table, &NEW, sizeof(uint16_t));
        }
        fputs(S, destFptr);
        C[0] = S[0];
        C[1] = '\0';

        char *cache = (char *)malloc(200 * sizeof(char));
        char *trans = (char *)ht_get_gen(table, &OLD, sizeof(uint16_t));
        if (trans == NULL)
        {
            strcpy(cache, OLDcache);
        }
        else
        {
            strcpy(cache, trans);
        }
        strcat(cache, C);
        uint16_t *key = (uint16_t *)malloc(sizeof(uint16_t));
        *key = codeCount;
        ht_set_gen(table, (void *)key, sizeof(uint16_t), cache);
        codeCount++;
        OLD = NEW;
    }

    uint16_t test = 128;
    printf("test: %s\n", (char *)ht_get_gen(table, &test, sizeof(uint16_t)));
}
