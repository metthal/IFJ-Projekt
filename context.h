#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdint.h>

struct SymbolTable;

typedef struct {
    struct SymbolTable *localTable;
    uint32_t stackTop;
    uint32_t defaultStart; //!< Starting index in Constants Table
    uint16_t argumentCount;
    uint16_t localVariableCount;
    uint16_t defaultCount;
} Context;

void initContext(Context *pt);
void deleteContext(Context *pt);

#endif
