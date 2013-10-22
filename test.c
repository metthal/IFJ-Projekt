#include "test.h"

TestEP testSuites[256];
uint8_t testSuiteCount = 0;
uint32_t testCountOk = 0, testCountFailed = 0, onlyFailed = 0;

// Here add test suite so test.c knows what test suites it can expect
TEST_SUITE(StringTests)
TEST_SUITE(ScannerTests)

int main(int argc, char **argv)
{
    if (argc == 2 && (strcmp(argv[1], "-f") == 0 || strcmp(argv[1], "--failed") == 0))
        onlyFailed = 1;

    // Register new test suite here if you want to run the test suite
    REGISTER_TEST_SUITE(StringTests)
    REGISTER_TEST_SUITE(ScannerTests)

    RUN_TEST_SUITES

    printfc(1, 33, "\nResult:\n");
    printf("Total tests passed: ");
    printfc(1, 32, "%38u\n", testCountOk);

    printf("Total tests failed: ");
    printfc(1, 31, "%38u\n", testCountFailed);

    return (testCountFailed > 0);
}
