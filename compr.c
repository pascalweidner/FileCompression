#include <stdio.h>
#include "huffmanCompression.h"

int main()
{
    huffmanEncode("/mnt/d/Development/Languages/C/FileCompression/files/test.txt");
    huffmanDecode("/mnt/d/Development/Languages/C/FileCompression/files/testHE.txt");

    return 0;
}