#include "context.h"
#include "ial.h"
#include <stdlib.h>
#include <string.h>

void initContext(Context *pt)
{
    memset(pt, 0, sizeof(Context));
    pt->localTable = newSymbolTable();
}

void deleteContext(Context *pt)
{
    if (pt != NULL) {
        freeSymbolTable(&pt->localTable);
    }
}
