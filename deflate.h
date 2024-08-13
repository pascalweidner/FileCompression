#ifndef DEFLATE_H_
#define DEFLATE_H_

#include <stdio.h>
#include <stdbool.h>

bool encodeDeflate(char filename[]);

bool decodeDeflate(char filename[]);

#endif