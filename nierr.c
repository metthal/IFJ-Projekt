#include "nierr.h"
/** Global interpret error variable. */
NiError niErr;

#ifdef DEBUG
#include <unistd.h>

#define STD_ERR_INT   2
#define NORMAL_COLOR  "\033[0m"
#define ERR_COLOR     "\033[1m\033[31m"

char errorPrinted = 1;

void printErrorF(const char *str)
{
    if (isatty(STD_ERR_INT)) {
        fprintf(stderr, ERR_COLOR "Error at %s:%d:%s: %s" NORMAL_COLOR,
                niErr.file, niErr.line, niErr.fun, str);
    }
    else {
        fprintf(stderr, "Error at %s:%d:%s: %s",
                niErr.file, niErr.line, niErr.fun, str);
    }
}

void printError()
{
    if (niErr.type == ERR_None || errorPrinted)
        return;


    switch (niErr.type) {
        case ERR_Unknown:
            printErrorF("Unknown error.\n");
            break;
        case ERR_LexFile:
            printErrorF("Scanner couldn't process file.\n");
            break;
        case ERR_Allocation:
            printErrorF("Allocation failed.\n");
            break;
        case ERR_Range:
            printErrorF("Index was outside of allowed range.\n");
            break;
        case ERR_Internal:
            printErrorF("Internal error.\n");
            break;
        case ERR_Convert:
            printErrorF("Conversion failed.\n");
            break;
        case ERR_Syntax:
            printErrorF("Syntax error.\n");
            break;
        case ERR_UndefFunction:
            printErrorF("Undefined function used.\n");
            break;
        case ERR_RedefFunction:
            printErrorF("Redefinition of function.\n");
            break;
        case ERR_UndefVariable:
            printErrorF("Undefined variable used.\n");
            break;
        case ERR_RedefParameter:
            printErrorF("Redefinition of function parameter.\n");
            break;
        case ERR_BadParamCount:
            printErrorF("Function called with wrong number of parameters.\n");
            break;
        case ERR_DefArgOrder:
            printErrorF("Non-default parameter appeared after default one.\n");
            break;
        case ERR_BadDefArg:
            printErrorF("Invalid default value of parameter.\n");
            break;
        case ERR_ISTGenerator:
            printErrorF("Instruction generator error.\n");
            break;
        case ERR_CycleControl:
            printErrorF("Break or continue used outside cycle.\n");
            break;
        case ERR_Substr:
            printErrorF("Substring parameters are wrong.\n");
            break;
        case ERR_OperandTypes:
            printf("Invalid operand type.\n");
        case ERR_DivideByZero:
            printf("Division by zero error.\n");
        default:
            printErrorF("Undocumented error.\n");
            break;
    }

    errorPrinted = 1;
}

#endif
