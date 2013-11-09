#include <stdlib.h>
#include <stdio.h>

#include "ial.h"
#include "nierr.h"

void initSymbolEntry(SymbolEntry *se);
void deleteSymbolEntry(SymbolEntry *se);
void copySymbolEntry(SymbolEntry *src, SymbolEntry *dest);

typedef SymbolEntry* SymbolEntryVectorIterator;
typedef const SymbolEntry* ConstSymbolEntryVectorIterator;

#define VECTOR_STRUCT_ITEM
#define VECTOR_ITEM SymbolEntry
#define VECTOR_ITERATOR SymbolEntryVectorIterator
#include "vector_template.h"
#undef VECTOR_ITERATOR
#undef VECTOR_ITEM
#undef VECTOR_STRUCT_ITEM

void initSymbolEntry(SymbolEntry *se)
{
    initSymbol(&(se->symbol));
    se->next = NULL;
    return;
}

void deleteSymbolEntry(SymbolEntry *se)
{
    deleteSymbol(&(se->symbol));
    se->next = NULL;
    return;
}

void copySymbolEntry(SymbolEntry *src, SymbolEntry *dest)
{
    dest->hash = src->hash;
    dest->next = src->next;
    copySymbol(&(src->symbol), &(dest->symbol));
    return;
}

uint32_t symbolTableHash(SymbolTable *st, String *key)
{
    uint32_t i, hash = 0;
    for ( i = 0; i < key->length - 1; ++i ) {
        hash = 33 * hash + (unsigned char)key->data[i];
    }
    if (st != NULL) {
        if ( hash > st->size ) {
            hash = hash % st->size;
        }
    }
    return hash;
}

void initSymbolTable(SymbolTable *st)
{
    st->table = malloc(sizeof(SymbolEntry*) * SYMBOL_TABLE_DEFAULT_SIZE);
    if (!st->table) {
        setError(ERR_NewFailed);
        return;
    }
    memset(st->table, 0, sizeof(SymbolEntry*) * SYMBOL_TABLE_DEFAULT_SIZE);
    st->vec = newSymbolEntryVector();
    if (!st->vec) {
        return;
    }
    st->size = SYMBOL_TABLE_DEFAULT_SIZE;
    st->count = 0;
}

void deleteSymbolTable(SymbolTable *st)
{
    free(st->table);
    st->table = NULL;
    freeSymbolEntryVector(&st->vec);
    st->vec = NULL;
    st->size = 0;
}

static inline void _symbolTableAddSymbolEntry(SymbolTable *st, SymbolEntry* se)
{
    uint32_t hash = se->hash % st->size;
    SymbolEntry *symbolEntry = st->table[hash];

    if (symbolEntry != NULL) {
        while (symbolEntry->next != NULL) {
            symbolEntry = symbolEntry->next;
        }
        symbolEntry->next = se;
    }
    else {
        st->table[hash] = se;
    }
}

void symbolTableResize(SymbolTable *st, uint32_t size)
{
    free(st->table);
    st->table = malloc(size * sizeof(SymbolEntry*));
    if (st->table) {
        memset(st->table, 0, size * sizeof(SymbolEntry*));
        for (uint32_t i = 0; i < st->vec->size; i++) {
            SymbolEntry* symbolEntry = (SymbolEntry*)st->vec->data + sizeof(SymbolEntry*) * i;
            if (!symbolEntry->symbol.key->data) {
                continue;
            }
            symbolEntry->next = NULL;
            _symbolTableAddSymbolEntry(st, symbolEntry);
        }
    }
    else {
        setError(ERR_NewFailed);
    }
}

static inline void symbolTableCheckIncrease(SymbolTable *st)
{
    if (st->count > (3 * st->size) / 4) {
        symbolTableResize(st, st->size * 2);
    }
}

Symbol* symbolTableFind(SymbolTable *st, String *key)
{
    uint32_t hash = symbolTableHash(st, key);
    SymbolEntry *symbolEntry = st->table[hash];
    if (symbolEntry != NULL) {
        while (stringCompare(key, symbolEntry->symbol.key) != 0) {
            if ((symbolEntry = symbolEntry->next) == NULL) {
                break;
            }
        }
    }
    return (Symbol*)symbolEntry;
}

Symbol* symbolTableAdd(SymbolTable *st, String *key)
{
    uint32_t fullHash = symbolTableHash(NULL, key),
                hash = fullHash % st->size;
    SymbolEntry *symbolEntry = st->table[hash], *newSymbolEntry;

    vectorPushDefaultSymbolEntry(st->vec);
    newSymbolEntry = vectorBack(st->vec);
    if (!newSymbolEntry) {
        return NULL;
    }
    newSymbolEntry->hash = fullHash;
    newSymbolEntry->symbol.key = key;

    if (symbolEntry != NULL) {
        while (stringCompare(key, symbolEntry->symbol.key) != 0) {
            SymbolEntry *previousSymbolEntry = symbolEntry;
            if ((symbolEntry = symbolEntry->next) == NULL) {
                previousSymbolEntry->next = newSymbolEntry;
                break;
            }
        }
    }
    else {
        st->table[hash] = newSymbolEntry;
    }

    if (symbolEntry == NULL) {
        st->count++;
        symbolTableCheckIncrease(st);
        return &(newSymbolEntry->symbol);
    } else {
        vectorPopSymbolEntry(st->vec);
        return NULL;
    }
}


uint32_t* stringSubstrSearchBuildTable(const char *str, uint32_t len)
{
    uint32_t *table = malloc(sizeof(uint32_t) * len);
    if (!table) {
        return table;
    }
    uint32_t tableIndex = 2, jumpIndex = 0;
    table[0] = table[1] = 0;

    while (tableIndex < len) {
        if (str[tableIndex - 1] == str[jumpIndex]) {
            table[tableIndex] = ++jumpIndex;
            tableIndex++;
        } else if (jumpIndex > 0) {
            jumpIndex = table[jumpIndex];
        } else {
            table[tableIndex] = 0;
            tableIndex++;
        }
    }

    return table;
}

static inline uint32_t _stringSubstrSearchSSO(const char *haystack, uint32_t haystackLen, const char *needle, uint32_t needleLen, uint32_t offset)
{
    uint32_t haystackIndex = offset, needleIndex = 0, result = haystackLen;
    uint32_t *table = stringSubstrSearchBuildTable(needle, needleLen);

    while (haystackIndex + needleIndex < haystackLen) {
        if (needle[needleIndex] == haystack[haystackIndex + needleIndex]) {
            if (needleIndex == needleLen - 1) {
                result = haystackIndex;
                break;
            }
            needleIndex++;
        } else {
            if (needleIndex > 0) {
                haystackIndex += needleIndex - table[needleIndex];
                needleIndex = table[needleIndex];
            } else {
                haystackIndex += needleIndex + 1;
            }
        }
    }
    
    free(table);
    return result;
}

uint32_t stringSubstrSearch(String *haystack, String *needle)
{
    return _stringSubstrSearchSSO(haystack->data, haystack->length - 1, needle->data, needle->length - 1, 0);
}

uint32_t stringSubstrSearchO(String *haystack, String *needle, uint32_t offset)
{
    return _stringSubstrSearchSSO(haystack->data, haystack->length - 1, needle->data, needle->length - 1, offset);
}

uint32_t stringSubstrSearchS(String *haystack, const char *needle, uint32_t needleLen)
{
    return _stringSubstrSearchSSO(haystack->data, haystack->length - 1, needle, needleLen, 0);
}

uint32_t stringSubstrSearchSO(String *haystack, const char *needle, uint32_t needleLen, uint32_t offset)
{
    return _stringSubstrSearchSSO(haystack->data, haystack->length - 1, needle, needleLen, offset);
}

uint32_t stringSubstrSearchSS(const char *haystack, uint32_t haystackLen, const char *needle, uint32_t needleLen)
{
    return _stringSubstrSearchSSO(haystack, haystackLen, needle, needleLen, 0);
}

uint32_t stringSubstrSearchSSO(const char *haystack, uint32_t haystackLen, const char *needle, uint32_t needleLen, uint32_t offset)
{
    return _stringSubstrSearchSSO(haystack, haystackLen, needle, needleLen, offset);
}
