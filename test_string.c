#include <string.h>

#include "test.h"
#include "string.h"

uint16_t stringCompareRaw(String *a, String *b)
{
    if ((a->size != b->size)
        || (a->length != b->length)) {
        return 0;
    }
    return !strcmp(a->data, b->data);
}

TEST_SUITE_START(StringTests);

int combinedTestResult = 0;

// newString*()
String *a = newString();
SHOULD_EQUAL_STR("newString()", a->data, "");
String *e = newStringSize(16);
if ((e->size = 16)
    && (!strcmp(e->data, ""))) {
    combinedTestResult = 1;
}
SHOULD_BE_TRUE("newStringSize()", combinedTestResult);
String *b = newStringS("Hello World!", 13);
SHOULD_EQUAL_STR("newStringS()", b->data, "Hello World!");

// stringResize()
stringResize(a, 50);
SHOULD_EQUAL("stringResize()", a->size, 50);

// stringClone()
String *c = stringClone(b);
combinedTestResult = 0;
if (stringCompareRaw(c, b)) {
    combinedTestResult = 1;
}
SHOULD_BE_TRUE("stringClone()", combinedTestResult);

// stringEmpty()
stringEmpty(c);
combinedTestResult = 0;
if ((c->length == 1)
    && (!strcmp(c->data, ""))) {
    combinedTestResult = 1;
}
SHOULD_BE_TRUE("stringEmpty()", combinedTestResult);

// stringPush()
stringEmpty(a);
for (uint8_t i = 'a'; i < 'a' + 26; i++) {
    stringPush(a, i);
}
SHOULD_EQUAL_STR("stringPush()", a->data, "abcdefghijklmnopqrstuvwxyz");

// stringPop()
for (uint8_t i = 0; i < 18; i++) {
    stringPop(a);
}
SHOULD_EQUAL_STR("stringPop()", a->data, "abcdefgh");

// stringCopy()
stringCopy(b, c);
SHOULD_EQUAL_STR("stringCopy()", b->data, c->data);

// stringSet()
stringSet(c, a);
SHOULD_EQUAL_STR("stringSet()", c->data, a->data);

// stringSetS()
stringSetS(c, "Bye World!", 11);
SHOULD_EQUAL_STR("stringSetS()", c->data, "Bye World!");

// stringAddS()
stringAddS(b, " ", 2);
SHOULD_EQUAL_STR("stringAddS()", b->data, "Hello World! ");

// stringAdd()
stringAdd(b, c);
SHOULD_EQUAL_STR("stringAdd()", b->data, "Hello World! Bye World!");

// freeString()
combinedTestResult = 0;
freeString(&a);
freeString(&b);
freeString(&c);
freeString(&e);
if (!(a || b || c || e)) {
    combinedTestResult = 1;
}
SHOULD_BE_TRUE("freeString()", combinedTestResult);

TEST_SUITE_END