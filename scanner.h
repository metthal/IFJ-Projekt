#ifndef SCANNER_H
#define SCANNER_H

#include "string.h"
#include "token.h"
#include "vector.h"
#include <stdio.h>

void scannerReset();
Vector* scannerScanFile(const char *fileName);
FILE* scannerOpenFile(const char *fileName);
Token* scannerGetToken();

#endif
