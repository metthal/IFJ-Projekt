#include "symbol.h"
#include <stdlib.h>
#include <string.h>

void initSymbol(Symbol *symbol)
{
    memset(symbol, 0, sizeof(Symbol));
}

void deleteSymbol(Symbol *symbol)
{
    if (symbol->type == ST_Function)
        freeFunction((Function**)&symbol->data);
    else
        freeVariableSymbolData((VariableSymbolData**)&symbol->data);
}

void copySymbol(Symbol *src, Symbol *dest)
{
    while(1);
}

VariableSymbolData* newVariableSymbolData()
{
    VariableSymbolData *tmp = malloc(sizeof(VariableSymbolData));
    memset(tmp, 0, sizeof(VariableSymbolData));
    return tmp;
}

void freeVariableSymbolData(VariableSymbolData **ppt)
{
    if (ppt != NULL) {
        free(*ppt);
        *ppt = NULL;
    }
}

Function* newFunction()
{
    Function *tmp = malloc(sizeof(Function));
    memset(tmp, 0, sizeof(Function));
    initContext(&tmp->context);
    return tmp;
}

void freeFunction(Function **ppt)
{
    if (ppt != NULL) {
        if (*ppt != NULL) {
            deleteContext(&(*ppt)->context);
        }
        free(*ppt);
        *ppt = NULL;
    }
}
