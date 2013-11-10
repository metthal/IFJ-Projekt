#include "context.h"
#include "ial.h"
#include <stdlib.h>

void initContext(Context *pt)
{
    pt->argumentCount = 0;
    pt->localVariableCount = 0;
    pt->localTable = newSymbolTable();
}

void deleteContext(Context *pt)
{
    if (pt != NULL) {
        freeSymbolTable(&pt->localTable);
    }
}
