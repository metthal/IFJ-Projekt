#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

typedef void (*TestEP)();

static const uint8_t TestNameLen = 48;
static const uint8_t TestResultPos = 50;
extern uint32_t testCountOk, testCountFailed, onlyFailed;

typedef enum
{
    TestOk,
    TestFailed
} TestResult;

static inline void printfc(uint8_t color, uint8_t style, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (!isatty(fileno(stdout)))
        vprintf(fmt, args);
    else {
        char buffer[256];
        vsprintf(buffer, fmt, args);
        printf("\033[%u;%um%s\033[00m", style, color, buffer);
    }
    va_end(args);
}

static inline void testResult(uint8_t result, const char *testName, const char *file, uint32_t line, const char *expr)
{
    char buffer[TestNameLen + 1];
    memset(buffer, 0, TestNameLen + 1);

    if (strlen(testName) >= TestNameLen) {
        strncpy(buffer, testName, TestNameLen - 1);
        buffer[TestNameLen] = '\0';
        testName = buffer;
    }

    if (!onlyFailed || result == TestFailed) {
        printf("%s", testName);

        uint8_t spaceCount = TestResultPos - strlen(testName);
        for (uint8_t i = 0; i < spaceCount; ++i)
            putchar(' ');
    }

    if (result == TestOk) {
        if (!onlyFailed) {
            printf("[ ");
            printfc(1, 32, "PASS");
            printf(" ]\n");
        }

        testCountOk++;
    }
    else {
        printf("[ ");
        printfc(1, 31, "FAIL");
        printf(" ]\n");
        printf("Test at %s:%u failed!\n\t%s\n\n", file, line, expr);
        testCountFailed++;
    }
}

#define REGISTER_TEST_SUITE(testSuiteName)      testSuites[testSuiteCount++] = &testSuiteName;
#define RUN_TEST_SUITES                         for (uint8_t i = 0; i < testSuiteCount; ++i) (*testSuites[i])();
#define TEST_SUITE(testSuiteName)               extern void testSuiteName();

#define TEST_SUITE_START(testSuiteName)         	\
    void testSuiteName() {							\
        printfc(1, 33, "\n%s:\n", #testSuiteName);	\
        printf("Starting test suite in %s...\n", __FILE__); \
        uint32_t testCountOkStart = testCountOk, testCountFailedStart = testCountFailed;

#define TEST_SUITE_END                          	\
        printf("Suite tests passed: ");				\
        printfc(1, 32, "%44u\n", testCountOk - testCountOkStart); \
        printf("Suite tests failed: ");				\
        printfc(1, 31, "%44u\n", testCountFailed - testCountFailedStart); \
    }

#define SHOULD_EQUAL(TestName, val1, val2)      \
        if (val1 == val2)                       \
            testResult(TestOk, TestName, __FILE__, __LINE__, NULL); \
        else                                    \
            testResult(TestFailed, TestName, __FILE__, __LINE__, "SHOULD_EQUAL("#val1 " == " #val2 ")");

#define SHOULD_NOT_EQUAL(TestName, val1, val2)  \
        if (val1 != val2)                       \
            testResult(TestOk, TestName, __FILE__, __LINE__, NULL); \
        else                                    \
            testResult(TestFailed, TestName, __FILE__, __LINE__, "SHOULD_NOT_EQUAL("#val1 " != " #val2 ")");

#define SHOULD_BE_GRT(TestName, val1, val2)     \
        if (val1 > val2)                        \
            testResult(TestOk, TestName, __FILE__, __LINE__, NULL); \
        else                                    \
            testResult(TestFailed, TestName, __FILE__, __LINE__, "SHOULD_BE_GRT("#val1 " > " #val2 ")");

#define SHOULD_BE_GRT_EQ(TestName, val1, val2)  \
        if (val1 >= val2)                       \
            testResult(TestOk, TestName, __FILE__, __LINE__, NULL); \
        else                                    \
            testResult(TestFailed, TestName, __FILE__, __LINE__, "SHOULD_BE_GRT_EQ("#val1 " >= " #val2 ")");

#define SHOULD_BE_LESS(TestName, val1, val2)    \
        if (val1 < val2)                        \
            testResult(TestOk, TestName, __FILE__, __LINE__, NULL); \
        else                                    \
            testResult(TestFailed, TestName, __FILE__, __LINE__, "SHOULD_BE_LESS("#val1 " < " #val2 ")");

#define SHOULD_BE_LESS_EQ(TestName, val1, val2) \
        if (val1 <= val2)                       \
            testResult(TestOk, TestName, __FILE__, __LINE__, NULL); \
        else                                    \
            testResult(TestFailed, TestName, __FILE__, __LINE__, "SHOULD_BE_LESS_EQ("#val1 " <= " #val2 ")");

#define SHOULD_BE_TRUE(TestName, expr)          \
        if (expr)                               \
            testResult(TestOk, TestName, __FILE__, __LINE__, NULL); \
        else                                    \
            testResult(TestFailed, TestName, __FILE__, __LINE__, "SHOULD_BE_TRUE("#expr")");

#define SHOULD_BE_FALSE(TestName, expr)         \
        if (!(expr))                            \
            testResult(TestOk, TestName, __FILE__, __LINE__, NULL); \
        else                                    \
            testResult(TestFailed, TestName, __FILE__, __LINE__, "SHOULD_BE_FALSE("#expr")");

#define SHOULD_EQUAL_STR(TestName, str1, str2)  \
        if (strcmp(str1, str2) == 0)            \
            testResult(TestOk, TestName, __FILE__, __LINE__, NULL); \
        else                                    \
            testResult(TestFailed, TestName, __FILE__, __LINE__, "SHOULD_EQUAL_STR(" #str1 " == " #str2 ")");

#define SHOULD_NOT_EQUAL_STR(TestName, str1, str2)  \
        if (strcmp(str1, str2) != 0)            \
            testResult(TestOk, TestName, __FILE__, __LINE__, NULL); \
        else                                    \
            testResult(TestFailed, TestName, __FILE__, __LINE__, "SHOULD_NOT_EQUAL_STR(" #str1 " != " #str2 ")");

#endif
