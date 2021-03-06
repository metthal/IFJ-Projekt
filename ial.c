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

/**
 * Hash function implementation.
 * If st is set, automatically returns hash for given table size.
 *
 * @param   st   SymbolTable
 * @param   key  Key to hash
 *
 * @return       Hash - See description
 */
uint32_t symbolTableHash(SymbolTable *st, const String *key)
{
    uint32_t i, hash = 0;
    for ( i = 0; i < key->length - 1; ++i ) {
        // Bernstein aka Djb2 hash function
        hash = ((hash << 5) + hash) + (unsigned char)key->data[i];
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

/**
 * Returns allocated and initialized SymbolTable.
 *
 * @return  SymbolTable pointer
 */
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

/**
 * Frees allocated SymbolTable.
 *
 * @param  st  Pointer to SymbolTable pointer
 */
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

/**
 * Initializes already allocated SymbolTable.
 *
 * @param  st  Pointer to SymbolTable
 */
void initSymbolTable(SymbolTable *st)
{
    _initSymbolTable(st);
}

/**
 * Deletes initialized SymbolTable.
 *
 * @param  st  Pointer to SymbolTable.
 */
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
    // Internal function to add index of SymbolEntry to SymbolTable after resize
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

/**
 * Resizes SymbolTable to given size.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param  st    SymbolTable to resize
 * @param  size  Size
 */
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
    // Check if load is bigger than 0.75
    if (st->count > (3 * st->size) / 4) {
        symbolTableResize(st, st->size * 2);
    }
}

/**
 * Returns pointer to Symbol if found, otherwise NULL.
 *
 * @param   st   SymbolTable to search in
 * @param   key  Key
 *
 * @return       Pointer to Symbol - See description
 */
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

/**
 * Tries to add new entry for given key. If succesfull returns pointer
 * to Symbol, otherwise NULL.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param   st   SymbolTable to add into
 * @param   key  Key
 *
 * @return       Pointer to Symbol - See description
 */
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

/**
 * Helper function to build jump table for KMP substring search.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param   str  C String to build table from
 * @param   len  Length of string
 *
 * @return       Pointer to Table
 */
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

static inline int64_t _stringSubstrSearchSSO(const char *haystack, uint32_t haystackLen, const char *needle, uint32_t needleLen, uint32_t offset)
{
    int64_t result = -1;
    uint32_t haystackIndex = offset, needleIndex = 0;
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

/**
 * Finds first occurence of needle in haystack, if not found returns -1.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param   haystack  String to search in
 * @param   needle    String to search for
 *
 * @return            Index of occurence - See description
 */
int64_t stringSubstrSearch(const String *haystack, const String *needle)
{
    return _stringSubstrSearchSSO(haystack->data, haystack->length - 1, needle->data, needle->length - 1, 0);
}

/**
 * Finds first occurence of needle in haystack from offset, if not found returns -1.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param   haystack  String to search in
 * @param   needle    String to search for
 * @param   offset    Index to start from
 *
 * @return            Index of occurence - See description
 */
int64_t stringSubstrSearchO(String *haystack, String *needle, uint32_t offset)
{
    return _stringSubstrSearchSSO(haystack->data, haystack->length - 1, needle->data, needle->length - 1, offset);
}

/**
 * See stringSubstrSearch()
 */
int64_t stringSubstrSearchS(String *haystack, const char *needle, uint32_t needleLen)
{
    return _stringSubstrSearchSSO(haystack->data, haystack->length - 1, needle, needleLen, 0);
}

/**
 * See stringSubstrSearchO()
 */
int64_t stringSubstrSearchSO(String *haystack, const char *needle, uint32_t needleLen, uint32_t offset)
{
    return _stringSubstrSearchSSO(haystack->data, haystack->length - 1, needle, needleLen, offset);
}

/**
 * See stringSubstrSearch()
 */
int64_t stringSubstrSearchSS(const char *haystack, uint32_t haystackLen, const char *needle, uint32_t needleLen)
{
    return _stringSubstrSearchSSO(haystack, haystackLen, needle, needleLen, 0);
}

/**
 * See stringSubstrSearchO()
 */
int64_t stringSubstrSearchSSO(const char *haystack, uint32_t haystackLen, const char *needle, uint32_t needleLen, uint32_t offset)
{
    return _stringSubstrSearchSSO(haystack, haystackLen, needle, needleLen, offset);
}

static inline void _stringCharSortMerge(char arr[], char temp[], uint32_t offset, uint32_t length)
{
    // Merges two intervals given by offset and length from temp[] into arr[]
    uint32_t leftIndex = 0,
        leftMax = offset,
        rightIndex = offset,
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
    // Recursively divides and merges temp[] into arr[]
    uint32_t offset = length / 2;
    if (length > 2) {
        stringCharSortDivide(temp, arr, offset);
        stringCharSortDivide(temp + offset, arr + offset, length - offset);
    }

    _stringCharSortMerge(arr, temp, offset, length);
}

/**
 * Sorts string in alphabetical order.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param  s  String to sort
 */
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
