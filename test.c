#include "test.h"

TestEP testSuites[256];
uint8_t testSuiteCount = 0;
uint32_t testCountOk = 0, testCountFailed = 0, onlyFailed = 0;


// Here add test suite so test.c knows what test suites it can expect
TEST_SUITE(StringTests)

int main(int argc, char **argv)
{
    if (argc == 2 && (strcmp(argv[1], "-f") == 0 || strcmp(argv[1], "--failed") == 0))
        onlyFailed = 1;

    // Register new test suite here if you want to run the test suite
    REGISTER_TEST_SUITE(StringTests)

    RUN_TEST_SUITES

    printf("\nTests passed: ");
    printfc(1, 32, "%44u\n", testCountOk);

    printf("Tests failed: ");
    printfc(1, 31, "%44u\n", testCountFailed);

    return (testCountFailed > 0);
}
