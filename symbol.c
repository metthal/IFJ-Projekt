/*
 * Project name:
 * Implementace interpretu imperativního jazyka IFJ13.
 *
 * Codename:
 * INI: Ni Interpreter
 *
 * Description:
 * https://wis.fit.vutbr.cz/FIT/st/course-files-st.php/course/IFJ-IT/projects/ifj2013.pdf
 *
 * Project's GitHub repository:
 * https://github.com/metthal/IFJ-Projekt
 *
 * Team:
 * Marek Milkovič   (xmilko01)
 * Lukáš Vrabec     (xvrabe07)
 * Ján Spišiak      (xspisi03)
 * Ivan Ševčík      (xsevci50)
 * Marek Bertovič   (xberto00)
 */

#include "symbol.h"
#include "nierr.h"
#include <stdlib.h>
#include <string.h>

void initValue(Value *value)
{
    memset(value, 0, sizeof(Value));
}

void deleteValue(Value *value)
{
    if (value->type == VT_String)
        deleteString(&(value->data.s));
}

void copyValue(const Value *src, Value *dest)
{
    dest->type = src->type;
    switch(src->type) {
        case VT_Null:
        case VT_Undefined:
            return;

        case VT_String:
            copyString(&(src->data.s), &(dest->data.s));
            break;

        default:
            dest->data = src->data;
    }
}

void tokenToValue(const Token *src, Value *dest)
{
    switch (src->type) {
        case STT_Number:
            dest->type = VT_Integer;
            dest->data.i = src->n;
            break;

        case STT_Double:
            dest->type = VT_Double;
            dest->data.d = src->d;
            break;

        case STT_String:
            dest->type = VT_String;
            initString(&(dest->data.s));
            copyString(&(src->str), &(dest->data.s));
            break;

        case STT_Bool:
            dest->type = VT_Bool;
            dest->data.b = src->n;
            break;

        case STT_Null:
            dest->type = VT_Null;
            break;

        default:
            setError(ERR_Convert);
            return;
    }
}

void initSymbol(Symbol *symbol)
{
    memset(symbol, 0, sizeof(Symbol));
}

void deleteSymbol(Symbol *symbol)
{
    if (symbol->type == ST_Function)
        freeFunction((Function**)&symbol->data);
    else
        freeVariable((Variable**)&symbol->data);
}

void copySymbol(const Symbol *src, Symbol *dest)
{
    copySymbolData(src->data, dest->data);
    dest->type = src->type;
    // Association with string in Token
    dest->key = src->key;
}

void copySymbolData(const SymbolData *src, SymbolData *dest)
{
    if (src != NULL && dest != NULL) {
        // Unsupported operation
        setError(ERR_Internal);
    }
}

Variable* newVariable()
{
    Variable *tmp = malloc(sizeof(Variable));
    memset(tmp, 0, sizeof(Variable));
    return tmp;
}

void freeVariable(Variable **ppt)
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
