#include <string.h>

#include "test.h"
#include "convert.h"

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

INT_TO_STRING_TEST("Negative", -1234567);

DOUBLE_TO_STRING_TEST("Zero", 0.0);

DOUBLE_TO_STRING_TEST("Generic", 1234567.8901234);

DOUBLE_TO_STRING_TEST("Negative", -1234567.8901234);

DOUBLE_TO_STRING_TEST("Exponent", 234.56e-34);

TEST_SUITE_END
