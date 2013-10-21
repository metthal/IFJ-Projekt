#include <string.h>

#include "test.h"
#include "string.h"

TEST_SUITE_START(StringTests);

// initString()
String x;
initString(&x);
SHOULD_EQUAL("initString() Length", x.length, 1);
SHOULD_EQUAL_STR("initString() Data", x.data, "");

// deleteString()
deleteString(&x);
SHOULD_EQUAL("delete() Size", x.size, 0);
SHOULD_EQUAL("delete() Length", x.length, 0);
SHOULD_EQUAL("delete() Data", x.data, NULL);

// initStringSize()
initStringSize(&x, 16);
SHOULD_EQUAL("initStringSize() Size", x.size, 16);
SHOULD_EQUAL("initStringSize() Length", x.length, 1);
SHOULD_EQUAL_STR("initStringSize() Data", x.data, "");
deleteString(&x);

// initStringS()
initStringS(&x, CSTR_ARG("Alalalalal"));
SHOULD_EQUAL("initString() Length", x.length, 11);
SHOULD_EQUAL_STR("initString() Data", x.data, "Alalalalal");
deleteString(&x);

// newString*()
String *a = newString();
SHOULD_EQUAL("newString() Length", a->length, 1);
SHOULD_EQUAL_STR("newString() Data", a->data, "");

String *b = newStringSize(16);
SHOULD_EQUAL("newStringSize() Size", b->size, 16);
SHOULD_EQUAL("newStringSize() Length", b->length, 1);
SHOULD_EQUAL_STR("newStringSize() Data", b->data, "");

String *c = newStringS(CSTR_ARG("Hello World!"));
SHOULD_EQUAL("newStringS() Length", c->length, 13);
SHOULD_EQUAL_STR("newStringS() Data", c->data, "Hello World!");

// stringResize()
stringResize(a, 50);
SHOULD_EQUAL("stringResize() Size", a->size, 50);

// stringClone()
String *d = stringClone(b);
SHOULD_EQUAL("stringClone() Length", d->length, b->length);
SHOULD_EQUAL_STR("stringClone() Data", d->data, b->data);

// stringEmpty()
stringEmpty(d);
SHOULD_EQUAL("stringEmpty() Length", d->length, 1);
SHOULD_EQUAL_STR("stringEmpty() Data", d->data, "");

// stringSet()
stringSet(a, c);
SHOULD_EQUAL("stringSet() Length", a->length, c->length);
SHOULD_EQUAL_STR("stringSet() Data", a->data, c->data);

// stringSetS()
stringSetS(c, CSTR_ARG("Bye Bye World!"));
SHOULD_EQUAL("stringSet() Length", c->length, 15);
SHOULD_EQUAL_STR("stringSetS() Data", c->data, "Bye Bye World!");

// stringPush()
stringEmpty(a);
for (uint8_t i = 'a'; i < 'a' + 26; i++) {
    stringPush(a, i);
}
SHOULD_EQUAL("stringPush() Length", a->length, 27);
SHOULD_EQUAL_STR("stringPush() Data", a->data, "abcdefghijklmnopqrstuvwxyz");

// stringPop()
stringSetS(a, CSTR_ARG("abcdefghijklmnopqrstuvwxyz"));
for (uint8_t i = 0; i < 18; i++) {
    stringPop(a);
}
SHOULD_EQUAL("stringPop() Length", a->length, 9);
SHOULD_EQUAL_STR("stringPop() Data", a->data, "abcdefgh");

// stringCopy()
stringCopy(b, c);
SHOULD_EQUAL("stringCopy() Length", b->length, c->length);
SHOULD_EQUAL_STR("stringCopy() Data", b->data, c->data);

// stringAddS()
stringSetS(b, CSTR_ARG("Hello World!"));
stringAddS(b, CSTR_ARG(" "));
SHOULD_EQUAL("stringAddS() Length", b->length, 14);
SHOULD_EQUAL_STR("stringAddS() Data", b->data, "Hello World! ");

// stringAdd()
stringSetS(b, CSTR_ARG("Hello World! "));
stringSetS(c, CSTR_ARG("Bye Bye World!"));
stringAdd(b, c);
SHOULD_EQUAL("stringAdd() Length", b->length, 28);
SHOULD_EQUAL_STR("stringAdd() Data", b->data, "Hello World! Bye Bye World!");

// stringLength()
stringSetS(b, CSTR_ARG("Hello World! Bye Bye World!"));
SHOULD_EQUAL("stringLength()", stringLength(b), 27);

// stringCompare()
stringSetS(a, CSTR_ARG("a"));
stringSetS(b, CSTR_ARG("ab"));
SHOULD_BE_LESS("stringCompare() 1", stringCompare(a, b), 0);
SHOULD_BE_GRT("stringCompare() 2", stringCompare(b, a), 0);
stringSetS(a, CSTR_ARG("ab"));
SHOULD_EQUAL("stringCompare() 3", stringCompare(b, a), 0);
stringEmpty(a);
SHOULD_BE_LESS("stringCompare() 4", stringCompare(a, b), 0);
SHOULD_BE_GRT("stringCompare() 5", stringCompare(b, a), 0);

// stringCompareS()
stringSetS(a, CSTR_ARG("a"));
SHOULD_EQUAL("stringCompareS() 1", stringCompareS(a, CSTR_ARG("a")), 0);
stringSetS(a, CSTR_ARG("ab"));
SHOULD_BE_GRT("stringCompareS() 2", stringCompareS(a, CSTR_ARG("a")), 0);
SHOULD_BE_LESS("stringCompareS() 3", stringCompareS(a, CSTR_ARG("abc")), 0);

// stringCompareSS()
SHOULD_EQUAL("stringCompareSS() 1", stringCompareSS(CSTR_ARG("a"), CSTR_ARG("a")), 0);
SHOULD_BE_GRT("stringCompareSS() 2", stringCompareSS(CSTR_ARG("ab"), CSTR_ARG("a")), 0);
SHOULD_BE_LESS("stringCompareSS() 3", stringCompareSS(CSTR_ARG("a"), CSTR_ARG("ab")), 0);

// freeString()
int combinedTestResult = 0;
freeString(&a);
freeString(&b);
freeString(&c);
freeString(&d);
if (!(a || b || c || d)) {
    combinedTestResult = 1;
}
SHOULD_BE_TRUE("freeString()", combinedTestResult);

TEST_SUITE_END