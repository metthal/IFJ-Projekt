#ifndef CONTEXT_H
#define CONTEXT_H

#include "symbol_table.h"

typedef struct{
    int argc;
    int localc;
    SymbolTable *localTable;
} Context;

void initContext(Context *pt);
void deleteContext(Context *pt);

#endif
