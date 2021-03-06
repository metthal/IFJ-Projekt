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

#include <string.h>

#include "test.h"
#include "ial.h"

TEST_SUITE_START(IalTests);

SymbolTable ST;
SymbolTable* st = &ST;
initSymbolTable(st);
String *key1;
String *key2;
Symbol *s1;
Symbol *s2;

// Basic test
key1 = newStringS(CSTR_ARG("sym1"));
key2 = newStringS(CSTR_ARG("sym2"));
s1 = symbolTableAdd(st, key1);
s2 = symbolTableAdd(st, key2);
if (s1 && s2) {
    s1 = symbolTableFind(st, key1);
    s2 = symbolTableFind(st, key2);
    if (s1 && s2) {
        SHOULD_EQUAL_STR("symbolTableFind() Basic 1", s1->key->data, key1->data);
        SHOULD_EQUAL_STR("symbolTableFind() Basic 2", s2->key->data, key2->data);
    } else {
        SHOULD_BE_TRUE("symbolTableFind() Basic ptr1", s1);
        SHOULD_BE_TRUE("symbolTableFind() Basic ptr2", s2);
    }
} else {
    SHOULD_BE_TRUE("symbolTableAdd() Basic ptr1", s1);
    SHOULD_BE_TRUE("symbolTableAdd() Basic ptr2", s2);
}
freeString(&key1);
freeString(&key2);

// Conflict test
key1 = newStringS(CSTR_ARG("AAKW"));
key2 = newStringS(CSTR_ARG("YYYY"));
s1 = symbolTableAdd(st, key1);
s2 = symbolTableAdd(st, key2);
if (s1 && s2) {
    s1 = symbolTableFind(st, key1);
    s2 = symbolTableFind(st, key2);
    if (s1 && s2) {
        SHOULD_EQUAL_STR("symbolTableFind() Conflict 1", s1->key->data, key1->data);
        SHOULD_EQUAL_STR("symbolTableFind() Conflict 2", s2->key->data, key2->data);
    } else {
        SHOULD_BE_TRUE("symbolTableFind() Conflict ptr1", s1);
        SHOULD_BE_TRUE("symbolTableFind() Conflict ptr2", s2);
    }
} else {
    SHOULD_BE_TRUE("symbolTableAdd() Conflict ptr1", s1);
    SHOULD_BE_TRUE("symbolTableAdd() Conflict ptr2", s2);
}
freeString(&key1);
freeString(&key2);

// Existing test
key1 = newStringS(CSTR_ARG("ABCD"));
s1 = symbolTableAdd(st, key1);
s2 = symbolTableAdd(st, key1);
SHOULD_BE_TRUE("symbolTableAdd() Existing First Time", s1);
if (s1) {
    s1 = symbolTableFind(st, key1);
    SHOULD_BE_TRUE("symbolTableFind() Existing ptr1", s1);
    if (s1) {
        SHOULD_EQUAL_STR("symbolTableFind() Existing", s1->key->data, key1->data);
    }
}
SHOULD_BE_FALSE("symbolTableAdd() Existing Second Time", s2);
freeString(&key1);

deleteSymbolTable(st);

uint32_t combinedTest = 1;
st = newSymbolTable();
// Resize test
#define ENTRIES_COUNT 256
String **keys = malloc(ENTRIES_COUNT * sizeof(String*));
key1 = newString();
for (size_t i = 0; i < ENTRIES_COUNT; i++) {
    keys[i] = newString();
    stringPush(key1, 'A' + (i % 25));
    copyString(key1, keys[i]);
    s1 = symbolTableAdd(st, keys[i]);
    if (!s1) {
        SHOULD_BE_TRUE("symbolTableAdd() Resize ptr", s1);
        combinedTest = 0;
    } else {
        s1->data = NULL;
    }
}

for (size_t i = 0; i < ENTRIES_COUNT; i++) {
    s1 = symbolTableFind(st, keys[i]);
    if (s1) {
        if (stringCompare(s1->key, keys[i]) != 0) {
            SHOULD_BE_TRUE("symbolTableFind() Resize value", stringCompare(s1->key, keys[i]) == 0);
            combinedTest = 0;
        }
    } else {
        // cppcheck hack
        {
            SHOULD_BE_TRUE("symbolTableFind() Resize ptr", s1);
        }
        combinedTest = 0;
    }
}

if (combinedTest == 1) {
    SHOULD_BE_TRUE("symbolTable 256 entries", combinedTest);
}

for (uint32_t i = 0; i < ENTRIES_COUNT; i++) {
    freeString(&keys[i]);
}
freeString(&key1);
free(keys);

freeSymbolTable(&st);

// stringSubstrSearch()
String *ps1 = newStringS(CSTR_ARG("Abrakadabra Kadabra!"));
String *ps2 = newStringS(CSTR_ARG("ada"));
SHOULD_EQUAL("stringSubstrSearch() 1", stringSubstrSearch(ps1, ps2), 5);
SHOULD_EQUAL("stringSubstrSearch() 2", stringSubstrSearch(ps1, ps1), 0);
SHOULD_EQUAL("stringSubstrSearchO()", stringSubstrSearchO(ps1, ps2, 6), 13);
freeString(&ps1);
freeString(&ps2);

// stringSubstrSearchS()
String *ps = newStringS(CSTR_ARG("Hello World! Bye Bye World!"));
SHOULD_EQUAL("stringSubstrSearchS()", stringSubstrSearchS(ps, CSTR_ARG("Bye Bye")), 13);
SHOULD_EQUAL("stringSubstrSearchS()", stringSubstrSearchS(ps, CSTR_ARG("Not Found")), -1);
SHOULD_EQUAL("stringSubstrSearchSO()", stringSubstrSearchSO(ps, CSTR_ARG("World"), 7), 21);
freeString(&ps);

// stringSubstrSearchSS()
SHOULD_EQUAL("stringSubstrSearchSS()", stringSubstrSearchSS(CSTR_ARG("Mama ma emu. Ema ma mamu."), CSTR_ARG("mamu")), 20);

SHOULD_EQUAL("stringSubstrSearchSSO() 1", stringSubstrSearchSSO(CSTR_ARG("Mama ma emu. Ema ma mamu."), CSTR_ARG("ma"), 0), 2);
SHOULD_EQUAL("stringSubstrSearchSSO() 2", stringSubstrSearchSSO(CSTR_ARG("Mama ma emu. Ema ma mamu."), CSTR_ARG("ma"), 3), 5);
SHOULD_EQUAL("stringSubstrSearchSSO() 3", stringSubstrSearchSSO(CSTR_ARG("Mama ma emu. Ema ma mamu."), CSTR_ARG("ma"), 6), 14);
SHOULD_EQUAL("stringSubstrSearchSSO() 4", stringSubstrSearchSSO(CSTR_ARG("Mama ma emu. Ema ma mamu."), CSTR_ARG("ma"), 15), 17);
SHOULD_EQUAL("stringSubstrSearchSSO() 5", stringSubstrSearchSSO(CSTR_ARG("Mama ma emu. Ema ma mamu."), CSTR_ARG("ma"), 18), 20);

// stringCharSort() Basic
ps = newStringS(CSTR_ARG("DCBA"));
stringCharSort(ps);
SHOULD_EQUAL_STR("stringCharSort() Basic1", ps->data, "ABCD");
freeString(&ps);

ps = newStringS(CSTR_ARG("ba"));
stringCharSort(ps);
SHOULD_EQUAL_STR("stringCharSort() Basic2", ps->data, "ab");
freeString(&ps);

ps = newStringS(CSTR_ARG("bca"));
stringCharSort(ps);
SHOULD_EQUAL_STR("stringCharSort() Basic3", ps->data, "abc");
freeString(&ps);

// stringCharSort() Normal
ps = newStringS(CSTR_ARG("dcbaEDCBA"));
stringCharSort(ps);
SHOULD_EQUAL_STR("stringCharSort() Normal", ps->data, "ABCDEabcd");
freeString(&ps);

// stringCharSort() Random
ps = newStringS(CSTR_ARG("gpMWkslFI"));
stringCharSort(ps);
SHOULD_EQUAL_STR("stringCharSort() Random", ps->data, "FIMWgklps");
freeString(&ps);

TEST_SUITE_END
