#ifndef SYMBOL_H
#define SYMBOL_H

#include "string.h"
#include "context.h"

typedef enum {
    VT_Integer,
    VT_Double,
    VT_String,
    VT_Bool,
    VT_Null,
} VariableType;

typedef union {
    int32_t i;
    double d;
    int8_t b;
    String s;
} VariableValue;

typedef struct {
    VariableValue value;
    VariableType type;
} Variable;

typedef struct {
    int32_t relativeIndex;
    uint32_t constantIndex;
    uint8_t declared;
    uint8_t constant;
} VariableSymbolData;

typedef struct {
    uint32_t functionAddressIndex;
    Context context;
} Function;

typedef enum {
    ST_Function,
    ST_Variable,
} SymbolType;

typedef union {
    VariableSymbolData var;
    Function func;
} SymbolData;

typedef struct {
    SymbolData *data;
    SymbolType type;
    const String *key;
} Symbol;

void initSymbol(Symbol *symbol);
void deleteSymbol(Symbol *symbol);
void copySymbol(Symbol *src, Symbol *dest);
VariableSymbolData* newVariableSymbolData();
void freeVariableSymbolData(VariableSymbolData **ppt);
Function* newFunction();
void freeFunction(Function **ppt);

#endif
