#include <stdlib.h>
#include <stdio.h>

#include "ial.h"
#include "symbol_entry_vector.h"
#include "nierr.h"

void initSymbolEntry(SymbolEntry *se)
{
    initSymbol(&(se->symbol));
    se->next = 0;
    return;
}

void deleteSymbolEntry(SymbolEntry *se)
{
    deleteSymbol(&(se->symbol));
    se->next = 0;
    return;
}

void copySymbolEntry(const SymbolEntry *src, SymbolEntry *dest)
{
    dest->hash = src->hash;
    dest->next = src->next;
    copySymbol(&(src->symbol), &(dest->symbol));
    return;
}

uint32_t symbolTableHash(SymbolTable *st, const String *key)
{
    uint32_t i, hash = 0;
    for ( i = 0; i < key->length - 1; ++i ) {
        hash = 33 * hash + (unsigned char)key->data[i];
    }
    if (st) {
        if ( hash > st->size ) {
            hash = hash % st->size;
        }
    }
    return hash;
}

static inline void* _initSymbolTable(SymbolTable *st)
{
    st->table = malloc(sizeof(*(st->table)) * SYMBOL_TABLE_DEFAULT_SIZE);
    if (!st->table) {
        setError(ERR_Allocation);
        return NULL;
    }
    memset(st->table, 0, sizeof(*(st->table)) * SYMBOL_TABLE_DEFAULT_SIZE);
    st->vec = newSymbolEntryVector();
    if (!st->vec) {
        free(st->table);
        // error should be set already in newSymbolEntryVector()
        return NULL;
    }
    // Insert dummy record so that indexes start from one
    vectorPushDefaultSymbolEntry(st->vec);
    st->size = SYMBOL_TABLE_DEFAULT_SIZE;
    st->count = 0;
    return st->table;
}

SymbolTable* newSymbolTable()
{
    SymbolTable *st = malloc(sizeof(SymbolTable));
    if (!st) {
        setError(ERR_Allocation);
        return NULL;
    }
    if (!_initSymbolTable(st)) {
        free(st);
        return NULL;
    }
    return st;
}

void freeSymbolTable(SymbolTable **st)
{
    if (st) {
        if (*st) {
            free((*st)->table);
            freeSymbolEntryVector(&((*st)->vec));
        }
        free(*st);
        *st = NULL;
    }
}

void initSymbolTable(SymbolTable *st)
{
    _initSymbolTable(st);
}

void deleteSymbolTable(SymbolTable *st)
{
    free(st->table);
    st->table = NULL;
    freeSymbolEntryVector(&st->vec);
    st->vec = NULL;
    st->size = 0;
}

static inline void _symbolTableAddSymbolEntry(SymbolTable *st, uint32_t nseIndex, uint32_t hash)
{
    if (st->table[hash]) {
        SymbolEntry *symbolEntry = vectorAt(st->vec, st->table[hash]);
        while (symbolEntry->next) {
            symbolEntry = vectorAt(st->vec, symbolEntry->next);
        }
        symbolEntry->next = nseIndex;
    }
    else {
        st->table[hash] = nseIndex;
    }
}

void symbolTableResize(SymbolTable *st, uint32_t size)
{
    free(st->table);
    st->table = malloc(size * sizeof(SymbolEntry*));
    if (st->table) {
        memset(st->table, 0, size * sizeof(SymbolEntry*));
        st->size = size;
        for (uint32_t i = 0; i < st->vec->size; i++) {
            SymbolEntry* symbolEntry = (SymbolEntry*)st->vec->data + i;
            if (!symbolEntry->symbol.key) {
                continue;
            }
            symbolEntry->next = 0;
            _symbolTableAddSymbolEntry(st, i, symbolEntry->hash % size);
        }
    }
    else {
        setError(ERR_Allocation);
    }
}

static inline void symbolTableCheckIncrease(SymbolTable *st)
{
    if (st->count > (3 * st->size) / 4) {
        symbolTableResize(st, st->size * 2);
    }
}

Symbol* symbolTableFind(SymbolTable *st, const String *key)
{
    uint32_t index = st->table[symbolTableHash(st, key)];
    SymbolEntry *symbolEntry = NULL;
    if (index) {
        symbolEntry = vectorAt(st->vec, index);
        while (stringCompare(key, symbolEntry->symbol.key) != 0) {
            if (!symbolEntry->next) {
                symbolEntry = NULL;
                break;
            }
            symbolEntry = vectorAt(st->vec, symbolEntry->next);
        }
    }
    return (Symbol*)symbolEntry;
}

Symbol* symbolTableAdd(SymbolTable *st, const String *key)
{
    uint32_t fullHash = symbolTableHash(NULL, key),
        hash = fullHash % st->size,
        nseIndex;
    SymbolEntry *newSymbolEntry;
    Symbol *newSymbol;

    vectorPushDefaultSymbolEntry(st->vec);
    newSymbolEntry = vectorBack(st->vec);
    if (!newSymbolEntry) {
        return NULL;
    }
    nseIndex = newSymbolEntry - (SymbolEntry*)st->vec->data;
    newSymbolEntry->hash = fullHash;
    newSymbolEntry->symbol.key = key;
    newSymbol = &(newSymbolEntry->symbol);

    if (st->table[hash]) {
        SymbolEntry *symbolEntry = vectorAt(st->vec, st->table[hash]);
        while (stringCompare(key, symbolEntry->symbol.key) != 0) {
            if (!symbolEntry->next) {
                symbolEntry->next = nseIndex;
                break;
            }
            symbolEntry = vectorAt(st->vec, symbolEntry->next);
        }
        if (symbolEntry->next != nseIndex) {
            newSymbol = NULL;
        }
    }
    else {
        st->table[hash] = nseIndex;
    }

    if (newSymbol) {
        st->count++;
        symbolTableCheckIncrease(st);
    } else {
        vectorPopSymbolEntry(st->vec);
    }

    return newSymbol;
}

uint32_t* stringSubstrSearchBuildTable(const char *str, uint32_t len)
{
    uint32_t *table = malloc(sizeof(uint32_t) * len);
    if (!table) {
        setError(ERR_Allocation);
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
    uint32_t haystackIndex = offset, needleIndex = 0, result = (uint32_t)-1;
    uint32_t *table = stringSubstrSearchBuildTable(needle, needleLen);
    if (!table) {
        return result;
    }

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

uint32_t stringSubstrSearch(const String *haystack, const String *needle)
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

void stringCharSortMerge(char arr[], char temp[], uint32_t length)
{
    uint32_t leftIndex = 0,
        leftMax = length / 2,
        rightIndex = length / 2,
        rightMax = length;

    while ((leftIndex < leftMax) && (rightIndex < rightMax)) {
        if (temp[leftIndex] < temp[rightIndex]) {
            *arr = temp[leftIndex++];
            arr++;
        }
        else {
            *arr = temp[rightIndex++];
            arr++;
        }
    }

    while(leftIndex < leftMax) {
        *arr = temp[leftIndex++];
        arr++;
    }

    while(rightIndex < rightMax) {
        *arr = temp[rightIndex++];
        arr++;
    }
}

void stringCharSortDivide(char arr[], char temp[], uint32_t length)
{
    uint32_t offset = length / 2;
    if (offset != 0) {
        stringCharSortDivide(temp, arr, offset);
        stringCharSortDivide(temp + offset, arr + offset, length - offset);
    }

    stringCharSortMerge(arr, temp, length);
}

void stringCharSort(const String *s)
{
    char *temp = malloc(sizeof(char) * (s->length - 1));
    if (!temp) {
        setError(ERR_Allocation);
        return;
    }
    memcpy(temp, s->data, s->length - 1);
    stringCharSortDivide(s->data, temp, s->length - 1);
    free(temp);
}
