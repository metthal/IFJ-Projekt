#ifndef SYMBOL_H
#define SYMBOL_H

#include "string.h"
#include "context.h"
#include "instruction.h"
#include "token.h"

typedef enum {
    VT_Undefined = 0,
    VT_Integer,
    VT_Double,
    VT_String,
    VT_Bool,
    VT_Null,
    VT_InstructionPtr,
    VT_StackPtr
} ValueType;

typedef union {
    String s;
    double d;
    int i;
    Instruction* ip;
    uint32_t sp;
    int8_t b;
} ValueData;

typedef struct {
    ValueData data;
    ValueType type;
} Value;

typedef struct {
    int32_t relativeIndex;
} Variable;

typedef struct {
    uint32_t functionAddressIndex;
    Context context;
} Function;

typedef enum {
    ST_Function,
    ST_Variable,
} SymbolType;

typedef union {
    Variable var;
    Function func;
} SymbolData;

typedef struct {
    SymbolData *data;
    SymbolType type;
    const String *key;
} Symbol;

void initValue(Value *value);
void deleteValue(Value *value);
void copyValue(Value *src, Value *dest);

/// Value must be in "after-initialization" state
void tokenToValue(const Token *src, Value *dest);

void initSymbol(Symbol *symbol);
void deleteSymbol(Symbol *symbol);
void copySymbol(Symbol *src, Symbol *dest);

Variable* newVariable();
void freeVariable(Variable **ppt);

Function* newFunction();
void freeFunction(Function **ppt);

#endif
