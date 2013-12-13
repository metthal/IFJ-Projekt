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

/**
 * @file symbol.h
 * @brief Declares numerous data types used through project.
 */

#ifndef SYMBOL_H
#define SYMBOL_H

#include "string.h"
#include "context.h"
#include "token.h"

typedef enum {
    VT_Undefined = 0,  //!< VT_Undefined
    VT_Integer,        //!< VT_Integer
    VT_Double,         //!< VT_Double
    VT_String,         //!< VT_String
    VT_Bool,           //!< VT_Bool
    VT_Null,           //!< VT_Null
    VT_StrongReference,//!< VT_StrongReference
    VT_WeakReference,  //!< VT_WeakReference
    VT_ConstReference, //!< VT_ConstReference
    VT_InstructionPtr, //!< VT_InstructionPtr
    VT_StackPtr        //!< VT_StackPtr
} ValueType;

/** Union that holds value's information data. */
typedef union {
    String s;
    double d;
    int i;
    const void* ip;
    uint32_t sp;
    int64_t ref;
    int8_t b;
} ValueData;

/** Structure that holds data of Value. */
typedef struct {
    ValueData data;
    ValueType type;
} Value;

/** Structure that holds data of Variable. */
typedef struct {
    int32_t relativeIndex;
} Variable;

/** Structure that holds data of Function. */
typedef struct {
    uint32_t functionAddressIndex;
    Context context;
} Function;

/** Type of symbol used to access correct union member. */
typedef enum {
    ST_Function,
    ST_Variable,
} SymbolType;

/** Union that holds data sector of Symbol. */
typedef union {
    Variable var;
    Function func;
} SymbolData;

/** Structure that hold data of Symbol. */
typedef struct {
    SymbolData *data;
    SymbolType type;
    const String *key;
} Symbol;

void initValue(Value *value);
void deleteValue(Value *value);
void copyValue(const Value *src, Value *dest);

/// Value must be in initialized state
void tokenToValue(const Token *src, Value *dest);

void initSymbol(Symbol *symbol);
void deleteSymbol(Symbol *symbol);
void copySymbol(const Symbol *src, Symbol *dest);
void copySymbolData(const SymbolData *src, SymbolData *dest);

Variable* newVariable();
void freeVariable(Variable **ppt);

Function* newFunction();
void freeFunction(Function **ppt);

#endif
