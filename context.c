#include "context.h"
#include <stdlib.h>

void initContext(Context *pt)
{
    pt->argc = 0;
    pt->localc = 0;
    pt->localTable = newSymbolTable();
}

void deleteContext(Context *pt)
{
    if (pt != NULL) {
        freeSymbolTable(&pt->localTable);
    }
}
