#ifndef IAL_H
#define IAL_H

#include <ctype.h>
#include <stdint.h>

#include "symbol.h"
#include "vector.h"
#include "string.h"

typedef struct SymbolEntryStruct {
    Symbol symbol;
    uint32_t hash;
    struct SymbolEntryStruct *next;
} SymbolEntry;

static uint16_t const SYMBOL_TABLE_DEFAULT_SIZE = 128;

typedef struct SymbolTable {
    SymbolEntry **table;
    Vector* vec;
    uint32_t size;
    uint32_t count;
} SymbolTable;

void initSymbolTable(SymbolTable *st);
void deleteSymbolTable(SymbolTable *st);

Symbol* symbolTableFind(SymbolTable *st, String *key);
Symbol* symbolTableAdd(SymbolTable *st, String *key);

uint32_t stringSubstrSearch(String *haystack, String *needle);
uint32_t stringSubstrSearchO(String *haystack, String *needle, uint32_t offset);
uint32_t stringSubstrSearchS(String *haystack, const char *needle, uint32_t needleLen);
uint32_t stringSubstrSearchSO(String *haystack, const char *needle, uint32_t needleLen, uint32_t offset);
uint32_t stringSubstrSearchSS(const char *haystack, uint32_t haystackLen, const char *needle, uint32_t needleLen);
uint32_t stringSubstrSearchSSO(const char *haystack, uint32_t haystackLen, const char *needle, uint32_t needleLen, uint32_t offset);

#endif
