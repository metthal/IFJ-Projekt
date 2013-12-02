#include <string.h>

#include "test.h"
#include "convert.h"
#include "nierr.h"

TEST_SUITE_START(ConvertTests);

String *s;
char buffer[64];

#define INT_TO_STRING_TEST(NAME, NUM) \
    s = intToString(NUM); \
    sprintf(buffer, "%d", NUM); \
    SHOULD_EQUAL_STR("intToString() "NAME, s->data, buffer); \
    freeString(&s);

#define DOUBLE_TO_STRING_TEST(NAME, NUM) \
    s = doubleToString(NUM); \
    sprintf(buffer, "%g", NUM); \
    SHOULD_EQUAL_STR("doubleToString() "NAME, s->data, buffer); \
    freeString(&s);

INT_TO_STRING_TEST("Zero", 0);

INT_TO_STRING_TEST("Generic", 1234567);

INT_TO_STRING_TEST("Negative", 0);

DOUBLE_TO_STRING_TEST("Zero", 0.0);

DOUBLE_TO_STRING_TEST("Generic", 1234567.8901234);

DOUBLE_TO_STRING_TEST("Negative", 0.0);

DOUBLE_TO_STRING_TEST("Exponent", 234.56e-34);

// String to X tests
#define STRING_TO_INT_TEST(NAME, STR, VAL) \
    s = newStringS(STR, strlen(STR));  \
    SHOULD_EQUAL("stringToInt() "NAME, stringToInt(s), VAL);  \
    freeString(&s);

#define STRING_TO_DOUBLE_TEST(NAME, STR, VAL) \
    s = newStringS(STR, strlen(STR));  \
    SHOULD_EQUAL("stringToDouble() "NAME, stringToDouble(s), VAL);  \
    freeString(&s);

STRING_TO_INT_TEST("Generic", "42", 42);

STRING_TO_INT_TEST("Negative", "-42", 0);

STRING_TO_INT_TEST("Whitespaces", " \t\n42", 42);

STRING_TO_INT_TEST("Trailing chars", "42abc", 42);

STRING_TO_INT_TEST("Chars", "abc42", 0);

STRING_TO_INT_TEST("Empty", "", 0);

STRING_TO_DOUBLE_TEST("Generic", "42.42", 42.42);

STRING_TO_DOUBLE_TEST("Negative", "-42.42", 0.0);

STRING_TO_DOUBLE_TEST("Integral", "42", 42.0);

STRING_TO_DOUBLE_TEST("Exponent", "42.42e+42", 42.42e+42);

STRING_TO_DOUBLE_TEST("Whitespaces", " \t\n42.42", 42.42);

clearError();
STRING_TO_DOUBLE_TEST("Trailing chars", "42.42abc", 42.42);
SHOULD_EQUAL("stringToDouble() Chars - Error Type", getError(), ERR_None);

clearError();
STRING_TO_DOUBLE_TEST("Chars", "abc42.42", 0.0);
SHOULD_EQUAL("stringToDouble() Chars - Error Type", getError(), ERR_None);

clearError();
STRING_TO_DOUBLE_TEST("Empty", " ", 0.0);
SHOULD_EQUAL("stringToDouble() Chars - Error Type", getError(), ERR_None);

clearError();
STRING_TO_DOUBLE_TEST("Wrong dec. point", "42.abc", 0.0);
SHOULD_EQUAL("stringToDouble() Chars - Error Type", getError(), ERR_Convert);

clearError();
STRING_TO_DOUBLE_TEST("Wrong exponent", "42.42e+abc", 0.0);
SHOULD_EQUAL("stringToDouble() Chars - Error Type", getError(), ERR_Convert);
clearError();

TEST_SUITE_END
