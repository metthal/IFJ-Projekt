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

#ifndef IAL_H
#define IAL_H

#include <ctype.h>
#include <stdint.h>

#include "symbol.h"
#include "vector.h"
#include "string.h"

typedef struct {
    Symbol symbol;
    uint32_t hash;
    uint32_t next;
} SymbolEntry;

void initSymbolEntry(SymbolEntry *se);
void deleteSymbolEntry(SymbolEntry *se);
void copySymbolEntry(const SymbolEntry *src, SymbolEntry *dest);

static uint16_t const SYMBOL_TABLE_DEFAULT_SIZE = 128;

typedef struct SymbolTable {
    uint32_t *table;
    Vector* vec;
    uint32_t size;
    uint32_t count;
} SymbolTable;


SymbolTable* newSymbolTable();
void freeSymbolTable(SymbolTable **st);
void initSymbolTable(SymbolTable *st);
void deleteSymbolTable(SymbolTable *st);

Symbol* symbolTableFind(SymbolTable *st, const String *key);
Symbol* symbolTableAdd(SymbolTable *st, const String *key);

int64_t stringSubstrSearch(const String *haystack, const String *needle);
int64_t stringSubstrSearchO(String *haystack, String *needle, uint32_t offset);
int64_t stringSubstrSearchS(String *haystack, const char *needle, uint32_t needleLen);
int64_t stringSubstrSearchSO(String *haystack, const char *needle, uint32_t needleLen, uint32_t offset);
int64_t stringSubstrSearchSS(const char *haystack, uint32_t haystackLen, const char *needle, uint32_t needleLen);
int64_t stringSubstrSearchSSO(const char *haystack, uint32_t haystackLen, const char *needle, uint32_t needleLen, uint32_t offset);

void stringCharSort(const String *s);

#endif
