#include "test.h"
#include <unistd.h>

TestEP testSuites[256];
uint8_t testSuiteCount = 0;
uint32_t testCountOk = 0, testCountFailed = 0;
TestFlags testFlags = None;

// Here add test suite so test.c knows what test suites it can expect
TEST_SUITE(StringTests)
TEST_SUITE(ScannerTests)
TEST_SUITE(TokenVectorTests)
TEST_SUITE(IalTests)

int main(int argc, char **argv)
{
    int32_t opt;
    while ((opt = getopt(argc, argv, "hfv")) != -1)
    {
        switch (opt) {
            case 'h':
                printf("Usage:\n");
                printf("    ini-test [-h] | [[-f] [-v]]\n\n");
                printf("    -h                     Print help\n");
                printf("    -f                     Show only failed tests\n");
                printf("    -v                     Verbose output\n");
                return 0;
            case 'f':
                testFlags |= OnlyFailed;
                break;
            case 'v':
                testFlags |= VerboseOut;
                break;
            default:
                return 1;
        }
    }

    if (!(testFlags & VerboseOut))
        freopen("/dev/null", "w", stderr); // redirects stderr into /dev/null

    // Register new test suite here if you want to run the test suite
    REGISTER_TEST_SUITE(StringTests)
    REGISTER_TEST_SUITE(ScannerTests)
    REGISTER_TEST_SUITE(TokenVectorTests)
    REGISTER_TEST_SUITE(IalTests)

    RUN_TEST_SUITES

    printfc(1, 33, "\nResult:\n");
    printf("Total tests passed: ");
    printfc(1, 32, "%38u\n", testCountOk);

    printf("Total tests failed: ");
    printfc(1, 31, "%38u\n", testCountFailed);

    return (testCountFailed > 0);
}
