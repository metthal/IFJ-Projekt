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

// freeString()
freeString(&a);
SHOULD_EQUAL("stringFree()", a, NULL);

// stringClone()
a = stringClone(c);
SHOULD_EQUAL("stringClone() Length", a->length, c->length);
SHOULD_EQUAL_STR("stringClone() Data", a->data, c->data);
freeString(&a);

// stringSubstr()
a = stringSubstr(c, 0, 5);
SHOULD_EQUAL("stringClone() Length", a->length, 6);
SHOULD_EQUAL_STR("stringClone() Data", a->data, "Hello");

// stringResize()
stringResize(a, 50);
SHOULD_EQUAL("stringResize() Size", a->size, 50);

// stringEmpty()
stringEmpty(a);
SHOULD_EQUAL("stringEmpty() Length", a->length, 1);
SHOULD_EQUAL_STR("stringEmpty() Data", a->data, "");

// copyString()
copyString(c, a);
SHOULD_EQUAL("copyString() Length", a->length, c->length);
SHOULD_EQUAL_STR("copyString() Data", a->data, c->data);

// copyStringS()
copyStringS(CSTR_ARG("Bye Bye World!"), c);
SHOULD_EQUAL("copyStringS() Length", c->length, 15);
SHOULD_EQUAL_STR("copyStringS() Data", c->data, "Bye Bye World!");

// stringPush()
stringEmpty(a);
for (uint8_t i = 'a'; i < 'a' + 26; i++) {
    stringPush(a, i);
}
for (uint8_t i = 'a'; i < 'a' + 26; i++) {
    stringPush(a, i);
}
SHOULD_EQUAL("stringPush() Length", a->length, 53);
SHOULD_EQUAL_STR("stringPush() Data", a->data, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");

// stringPop() Resize
copyStringS(CSTR_ARG("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"), a);
for (uint8_t i = 0; i < 35; i++) {
    stringPop(a);
}
SHOULD_EQUAL("stringPop() Resize Length", a->length, 18);
SHOULD_EQUAL_STR("stringPop() Resize Data", a->data, "abcdefghijklmnopq");
// stringPop() Minimal
copyStringS(CSTR_ARG("abcdefghijklmnopq"), a);
for (uint8_t i = 0; i < 13; i++) {
    stringPop(a);
}
SHOULD_EQUAL("stringPop() Minimal Length", a->length, 5);
SHOULD_EQUAL_STR("stringPop() Minimal Data", a->data, "abcd");

// copyString()
copyString(c, b);
SHOULD_EQUAL("copyString() Length", b->length, c->length);
SHOULD_EQUAL_STR("copyString() Data", b->data, c->data);

// stringAddS()
copyStringS(CSTR_ARG("Hello World!"), b);
stringAddS(b, CSTR_ARG(" "));
SHOULD_EQUAL("stringAddS() Length", b->length, 14);
SHOULD_EQUAL_STR("stringAddS() Data", b->data, "Hello World! ");

// stringAdd()
copyStringS(CSTR_ARG("Hello World! "), b);
copyStringS(CSTR_ARG("Bye Bye World!"), c);
stringAdd(b, c);
SHOULD_EQUAL("stringAdd() Length", b->length, 28);
SHOULD_EQUAL_STR("stringAdd() Data", b->data, "Hello World! Bye Bye World!");

stringAdd(c, c);
SHOULD_EQUAL("stringAdd() Same String Length", c->length, 29);
SHOULD_EQUAL_STR("stringAdd() Same String Data", c->data, "Bye Bye World!Bye Bye World!");

// stringLength()
copyStringS(CSTR_ARG("Hello World! Bye Bye World!"), b);
SHOULD_EQUAL("stringLength()", stringLength(b), 27);

// stringCompare()
copyStringS(CSTR_ARG("a"), a);
copyStringS(CSTR_ARG("ab"), b);
SHOULD_BE_LESS("stringCompare() 1", stringCompare(a, b), 0);
SHOULD_BE_GRT("stringCompare() 2", stringCompare(b, a), 0);
copyStringS(CSTR_ARG("ab"), a);
SHOULD_EQUAL("stringCompare() 3", stringCompare(b, a), 0);
stringEmpty(a);
SHOULD_BE_LESS("stringCompare() 4", stringCompare(a, b), 0);
SHOULD_BE_GRT("stringCompare() 5", stringCompare(b, a), 0);

// stringCompareS()
copyStringS(CSTR_ARG("a"), a);
SHOULD_EQUAL("stringCompareS() 1", stringCompareS(a, CSTR_ARG("a")), 0);
copyStringS(CSTR_ARG("ab"), a);
SHOULD_BE_GRT("stringCompareS() 2", stringCompareS(a, CSTR_ARG("a")), 0);
SHOULD_BE_LESS("stringCompareS() 3", stringCompareS(a, CSTR_ARG("abc")), 0);

// stringCompareSS()
SHOULD_EQUAL("stringCompareSS() 1", stringCompareSS(CSTR_ARG("a"), CSTR_ARG("a")), 0);
SHOULD_BE_GRT("stringCompareSS() 2", stringCompareSS(CSTR_ARG("ab"), CSTR_ARG("a")), 0);
SHOULD_BE_LESS("stringCompareSS() 3", stringCompareSS(CSTR_ARG("a"), CSTR_ARG("ab")), 0);

// cleanup
freeString(&a);
freeString(&b);
freeString(&c);

TEST_SUITE_END