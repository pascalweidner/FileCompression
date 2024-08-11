#ifndef LZW_H_
#define LZW_H_

#include <stdbool.h>
#include <stdio.h>

bool lzwEncode(char filename[]);
bool lzwDecode(char filename[]);

#endif