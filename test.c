#include "test.h"

#define REGISTER_TEST_SUITE(testSuiteName)      testSuites[testSuiteCount++] = &testSuiteName;
#define RUN_TEST_SUITES                         for (uint8_t i = 0; i < testSuiteCount; ++i) (*testSuites[i])();
TestEP testSuites[256];
uint8_t testSuiteCount = 0;

// Here add test suite so test.c knows what test suites it can expect

int main()
{
    // Register new test suite here if you want to run the test suite

    RUN_TEST_SUITES
    return 0;
}
