#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdint.h>

struct SymbolTable;

typedef struct{
    struct SymbolTable *localTable;
    uint16_t argumentCount;
    uint16_t localVariableCount;
} Context;

void initContext(Context *pt);
void deleteContext(Context *pt);

#endif
