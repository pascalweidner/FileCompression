#include <stdio.h>
#include "lzw.h"
#include "huffmanCompression.h"

int main()
{
    // huffmanEncode("/mnt/d/Development/Languages/C/FileCompression/files/test.txt");
    // huffmanDecode("/mnt/d/Development/Languages/C/FileCompression/files/testHE.txt");
    lzwEncode("/mnt/d/Development/Languages/C/FileCompression/files/test.txt");
    return 0;
}