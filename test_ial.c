#include <string.h>

#include "test.h"
#include "ial.h"

TEST_SUITE_START(IalTests);

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
