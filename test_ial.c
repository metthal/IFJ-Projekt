#include <string.h>

#include "test.h"
#include "ial.h"

TEST_SUITE_START(IalTests);

SymbolTable st;
initSymbolTable(&st);

String *key1 = newStringS(CSTR_ARG("sym1"));
String *key2 = newStringS(CSTR_ARG("sym2"));
Symbol *s1 = symbolTableAdd(&st, key1);
Symbol *s2 = symbolTableAdd(&st, key2);
if (s1 && s2) {
    s1 = symbolTableFind(&st, key1);
    s2 = symbolTableFind(&st, key2);
    if (s1 && s2) {
        printf("%s %s\n", s1->key->data, key1->data);
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

key1 = newStringS(CSTR_ARG("AAKW"));
key2 = newStringS(CSTR_ARG("YYYY"));
s1 = symbolTableAdd(&st, key1);
s2 = symbolTableAdd(&st, key2);
if (s1 && s2) {
    s1 = symbolTableFind(&st, key1);
    s2 = symbolTableFind(&st, key2);
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

deleteSymbolTable(&st);

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
SHOULD_EQUAL("stringSubstrSearchSO()", stringSubstrSearchSO(ps, CSTR_ARG("World"), 7), 21);
freeString(&ps);

// stringSubstrSearchSS()
SHOULD_EQUAL("stringSubstrSearchSS()", stringSubstrSearchSS(CSTR_ARG("Mama ma emu. Ema ma mamu."), CSTR_ARG("mamu")), 20);

SHOULD_EQUAL("stringSubstrSearchSSO() 1", stringSubstrSearchSSO(CSTR_ARG("Mama ma emu. Ema ma mamu."), CSTR_ARG("ma"), 0), 2);
SHOULD_EQUAL("stringSubstrSearchSSO() 2", stringSubstrSearchSSO(CSTR_ARG("Mama ma emu. Ema ma mamu."), CSTR_ARG("ma"), 3), 5);
SHOULD_EQUAL("stringSubstrSearchSSO() 3", stringSubstrSearchSSO(CSTR_ARG("Mama ma emu. Ema ma mamu."), CSTR_ARG("ma"), 6), 14);
SHOULD_EQUAL("stringSubstrSearchSSO() 4", stringSubstrSearchSSO(CSTR_ARG("Mama ma emu. Ema ma mamu."), CSTR_ARG("ma"), 15), 17);
SHOULD_EQUAL("stringSubstrSearchSSO() 5", stringSubstrSearchSSO(CSTR_ARG("Mama ma emu. Ema ma mamu."), CSTR_ARG("ma"), 18), 20);

TEST_SUITE_END
