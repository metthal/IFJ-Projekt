#include "test.h"
#include "parser.h"
#include "token.h"
#include "token_vector.h"
#include "instruction_vector.h"
#include "value_vector.h"
#include "address_vector.h"
#include "scanner.h"

extern Vector *mainInstructions;
extern Vector *constantsTable;
extern Vector *functionsInstructions;
extern Vector *addressTable;

static uint8_t instTestFailed = 1;
static Vector *expectedMainInstrVector = NULL;
static Vector *expectedFuncInstrVector = NULL;
static Vector *tokenVector = NULL;
static ConstInstructionVectorIterator end1, end2, itr1, itr2;

#define ADD_MAIN_INSTRUCTION(opcode, R, A, B)   \
{                                               \
    vectorPushDefaultInstruction(expectedMainInstrVector);   \
    Instruction *inst = vectorAt(expectedMainInstrVector, vectorSize(expectedMainInstrVector) - 1); \
    inst->code = opcode;                    \
    inst->a = A;                            \
    inst->b = B;                            \
    inst->res = R;                          \
}

#define ADD_FUNC_INSTRUCTION(opcode, R, A, B)   \
{                                               \
    vectorPushDefaultInstruction(expectedFuncInstrVector);   \
    Instruction *inst = vectorAt(expectedFuncInstrVector, vectorSize(expectedFuncInstrVector) - 1); \
    inst->code = opcode;                    \
    inst->a = A;                            \
    inst->b = B;                            \
    inst->res = R;                          \
}

#define INSTRUCTIONS_EQ(inst1, inst2)       \
    ((inst1->code == inst2->code) && (inst1->a == inst2->a) && (inst1->b == inst2->b) && (inst1->res == inst2->res))

#define TEST_RESULT(file, testName, failed, funcFailed, errCode, constSize, addrSize)   \
    tokenVector = scannerScanFile(file);                                                \
    if (tokenVector)                                                                    \
        parse(tokenVector, 1);                                                          \
    SHOULD_EQUAL("Parser - " testName " - error code", getError(), errCode);            \
    if (tokenVector) {                                                                  \
        testInstructions(mainInstructions, expectedMainInstrVector);                    \
        SHOULD_EQUAL("Parser - " testName " - instructions", instTestFailed, failed);   \
        instTestFailed = 1;                                                             \
        testInstructions(functionsInstructions, expectedFuncInstrVector);               \
        SHOULD_EQUAL("Parser - " testName " - function instructions", instTestFailed, funcFailed);          \
        SHOULD_EQUAL("Parser - " testName " - const table size", vectorSize(constantsTable), constSize);    \
        SHOULD_EQUAL("Parser - " testName " - addr table size", vectorSize(addressTable), addrSize);        \
    }                                                                                                       \
    CLEANUP;

#define CLEANUP                             \
    clearError();                           \
    instTestFailed = 1;                     \
    vectorClearInstruction(expectedMainInstrVector);  \
    vectorClearInstruction(expectedFuncInstrVector);  \
    if (constantsTable)                         \
        freeValueVector(&constantsTable);       \
    if (addressTable)                           \
        freeInstructionPtrVector(&addressTable);\
    if (mainInstructions)                       \
        freeInstructionVector(&mainInstructions);   \
    if (functionsInstructions)                  \
        freeInstructionVector(&functionsInstructions);

#define SAVE_STATE(inFile, outFile)                                                     \
    tokenVector = scannerScanFile(inFile);                                              \
    if (tokenVector)                                                                    \
        parse(tokenVector, 1);                                                          \
    if (tokenVector) {                                                                  \
        printState(outFile);                                                            \
    }                                                                                   \
    CLEANUP;

void testInstructions(Vector *genInstrVector, Vector *expectedInstrVector)
{
    if (!genInstrVector) {
        instTestFailed = 0;
        return;
    }

    end1 = vectorEndInstruction(genInstrVector);
    end2 = vectorEndInstruction(expectedInstrVector);
    itr1 = vectorBeginInstruction(genInstrVector);
    itr2 = vectorBeginInstruction(expectedInstrVector);
    uint8_t allOk = 1;

    if (vectorSize(genInstrVector) != vectorSize(expectedInstrVector))
        return;

    uint32_t count = 0;
    for ( ; (itr1 != end1) && (itr2 != end2); ++itr1, ++itr2, ++count) {
        if (!INSTRUCTIONS_EQ(itr1, itr2)) {
            // COMMENT OUT THIS LINE IF YOU WANT TO COMPARE INSTRUCTIONS
            printf("(Generated/Expected) (%d) %d(RES: %d, A: %d, B: %d) / %d(RES: %d, A: %d, B: %d)\n", count, itr1->code, itr1->res, itr1->a, itr1->b, itr2->code, itr2->res, itr2->a, itr2->b);
            allOk = 0;
        }
    }

    if (allOk)
        instTestFailed = 0;
}

void printInstruction(FILE *file, int line, Instruction *ist)
{
    fprintf(file, "%d:\t", line);
    switch (ist->code) {
        case IST_Noop:
            fprintf(file, "!!! %11s", "Noop"); break;
        case IST_Mov:
            fprintf(file, "%15s", "Mov"); break;
        case IST_Jmp:
            fprintf(file, "%15s", "Jmp"); break;
        case IST_Jmpz:
            fprintf(file, "%15s", "Jmpz"); break;
        case IST_Push:
            fprintf(file, "%15s", "Push"); break;
        case IST_PushC:
            fprintf(file, "%15s", "PushC"); break;
        case IST_Reserve:
            fprintf(file, "%15s", "Reserve"); break;
        case IST_Pop:
            fprintf(file, "%15s", "Pop"); break;
        case IST_ClearExpr:
            fprintf(file, "%15s", "ClearExpr"); break;
        case IST_Call:
            fprintf(file, "%15s", "Call"); break;
        case IST_Return:
            fprintf(file, "%15s", "Return"); break;
        case IST_Nullify:
            fprintf(file, "%15s", "Nullify"); break;
        case IST_BoolVal:
            fprintf(file, "%15s", "Boolval"); break;
        case IST_DoubleVal:
            fprintf(file, "%15s", "Doubleval"); break;
        case IST_FindString:
            fprintf(file, "%15s", "FindString"); break;
        case IST_GetString:
            fprintf(file, "%15s", "GetString"); break;
        case IST_GetSubstring:
            fprintf(file, "%15s", "GetSubstring"); break;
        case IST_IntVal:
            fprintf(file, "%15s", "IntVal"); break;
        case IST_PutString:
            fprintf(file, "%15s", "PutString"); break;
        case IST_SortString:
            fprintf(file, "%15s", "SortString"); break;
        case IST_StrLen:
            fprintf(file, "%15s", "StrLen"); break;
        case IST_StrVal:
            fprintf(file, "%15s", "StrVal"); break;
        case IST_Break:
            fprintf(file, "!!! %11s", "Break"); break;
        case IST_Continue:
            fprintf(file, "!!! %11s", "Continue"); break;
        case IST_Add:
            fprintf(file, "%15s", "Add"); break;
        case IST_Subtract:
            fprintf(file, "%15s", "Subtract"); break;
        case IST_Multiply:
            fprintf(file, "%15s", "Multiply"); break;
        case IST_Divide:
            fprintf(file, "%15s", "Divide"); break;
        case IST_Concat:
            fprintf(file, "%15s", "Concat"); break;
        case IST_Equal:
            fprintf(file, "%15s", "Equal"); break;
        case IST_NotEqual:
            fprintf(file, "%15s", "NotEqual"); break;
        case IST_Less:
            fprintf(file, "%15s", "Less"); break;
        case IST_LessEq:
            fprintf(file, "%15s", "LessEq"); break;
        case IST_Greater:
            fprintf(file, "%15s", "Greater"); break;
        case IST_GreaterEq:
            fprintf(file, "%15s", "GreaterEq"); break;
        case IST_And:
            fprintf(file, "%15s", "And"); break;
        case IST_Or:
            fprintf(file, "%15s", "Or"); break;
        case IST_Not:
            fprintf(file, "%15s", "Not"); break;
    }
    fprintf(file, "    %5d %5d %5d\n", ist->res, ist->a, ist->b);
}

void printState(char *fileName)
{
    FILE *file = fopen(fileName, "w");

    uint32_t atSize = vectorSize(addressTable);
    fprintf(file, "Address Table (%d):\n", atSize);
    for (uint32_t i = 0; i < atSize; i++) {
        fprintf(file, "%d:\t%ld\n", i, (size_t)(*(InstructionPtr*)vectorAt(addressTable, i)));
    }

    uint32_t ctSize = vectorSize(constantsTable);
    ValueVectorIterator ctIt = vectorBeginValue(constantsTable);
    fprintf(file, "\nConst Table (%d):\n", ctSize);
    for (uint32_t i = 0; i < ctSize; i++, ctIt++) {
        fprintf(file, "%d:\t", i);
        switch(ctIt->type) {
            case VT_Undefined:
                fprintf(file, "%s\n", "U");
                break;

            case VT_Integer:
                fprintf(file, "%s\t%d\n", "I", ctIt->data.i);
                break;

            case VT_Double:
                fprintf(file, "%s\t%g\n", "D", ctIt->data.d);
                break;

            case VT_String:
                fprintf(file, "%s\t\"%s\"\n", "S", ctIt->data.s.data);
                break;

            case VT_Bool:
                fprintf(file, "%s\t%d\n", "B", ctIt->data.b);
                break;

            case VT_Null:
                fprintf(file, "%s\n", "N");
                break;

            case VT_Reference:
                fprintf(file, "%s\t%d\n", "R", ctIt->data.ref);
                break;

            case VT_ConstReference:
                fprintf(file, "%s\t%d\n", "CR", ctIt->data.ref);
                break;

            case VT_InstructionPtr:
                fprintf(file, "!!! %s\t%lu\n", "IP", (size_t)ctIt->data.ip);
                break;

            case VT_StackPtr:
                fprintf(file, "!!! %s\t%d\n", "SP", ctIt->data.sp);
                break;
        }
    }

    uint32_t miSize = vectorSize(mainInstructions);
    InstructionVectorIterator miIt = vectorBeginInstruction(mainInstructions);
    fprintf(file, "\nMain Instructions (%d):\n", miSize);
    for (uint32_t i = 0; i < miSize; i++, miIt++) {
        printInstruction(file, i, miIt);
    }

    uint32_t fiSize = vectorSize(functionsInstructions);
    InstructionVectorIterator fiIt = vectorBeginInstruction(functionsInstructions);
    fprintf(file, "\nFunctions Instructions (%d):\n", fiSize);
    for (uint32_t i = 0; i < fiSize; i++, fiIt++) {
        printInstruction(file, i, fiIt);
    }

    fclose(file);
}

TEST_SUITE_START(ParserTests)

expectedMainInstrVector = newInstructionVector();
if (getError())
    return;

expectedFuncInstrVector = newInstructionVector();
if (getError())
    return;

/*************************************************************************************
************************           TEST AREA               ***************************
*************************************************************************************/
uint32_t localVarStart = 0;
uint32_t exprStart = 0;

// Parser - missing PHP tag
// expected instructions
// none

// run tests
TEST_RESULT("sources/emptyfail.ifj", "missing PHP tag", 0, 0, ERR_Syntax, 0, 0);

// Parser - empty
// expected instructions
exprStart = 2;
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, exprStart, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, -1, 0);
ADD_MAIN_INSTRUCTION(IST_Return, 0, 0, 0);

// run tests
TEST_RESULT("sources/empty.ifj", "empty", 0, 0, ERR_None, 0, 0);

// Parser - Assign
// expected instructions
localVarStart = 2;
exprStart = 4;
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, exprStart, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, 1, 0);
// $a = 10;
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 0, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $b = $a;
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 1, localVarStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// end
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, -1, 0);
ADD_MAIN_INSTRUCTION(IST_Return, 0, 0, 0);

// run tests
TEST_RESULT("sources/assign.ifj", "assign", 0, 0, ERR_None, 1, 0);

// Parser - expressions
// expected instructions
localVarStart = 2;
exprStart = localVarStart + 14;
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, exprStart, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, 1, 0);
// $a = 10;
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 0, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $b = 10.0;
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 1, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $c = $a + $b;
ADD_MAIN_INSTRUCTION(IST_Add, exprStart + 0, localVarStart + 0, localVarStart + 1);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 2, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $d = $a - $b;
ADD_MAIN_INSTRUCTION(IST_Subtract, exprStart + 0, localVarStart + 0, localVarStart + 1);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 3, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $e = $a * $b;
ADD_MAIN_INSTRUCTION(IST_Multiply, exprStart + 0, localVarStart + 0, localVarStart + 1);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 4, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $f = $a / $b;
ADD_MAIN_INSTRUCTION(IST_Divide, exprStart + 0, localVarStart + 0, localVarStart + 1);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 5, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $g = $a === $b;
ADD_MAIN_INSTRUCTION(IST_Equal, exprStart + 0, localVarStart + 0, localVarStart + 1);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 6, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $h = $a !== $b;
ADD_MAIN_INSTRUCTION(IST_NotEqual, exprStart + 0, localVarStart + 0, localVarStart + 1);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 7, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $i = $c > $d;
ADD_MAIN_INSTRUCTION(IST_Greater, exprStart + 0, localVarStart + 2, localVarStart + 3);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 8, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $j = $e < $f;
ADD_MAIN_INSTRUCTION(IST_Less, exprStart + 0, localVarStart + 4, localVarStart + 5);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 9, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $k = $c >= $d;
ADD_MAIN_INSTRUCTION(IST_GreaterEq, exprStart + 0, localVarStart + 2, localVarStart + 3);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 10, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $l = $e <= $f;
ADD_MAIN_INSTRUCTION(IST_LessEq, exprStart + 0, localVarStart + 4, localVarStart + 5);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 11, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $m = (($a + $b) * $a + $b - $c) <= 190;
ADD_MAIN_INSTRUCTION(IST_Add, exprStart + 0, localVarStart + 0, localVarStart + 1);
ADD_MAIN_INSTRUCTION(IST_Multiply, exprStart + 1, exprStart + 0, localVarStart + 0);
ADD_MAIN_INSTRUCTION(IST_Add, exprStart + 2, exprStart + 1, localVarStart + 1);
ADD_MAIN_INSTRUCTION(IST_Subtract, exprStart + 3, exprStart + 2, localVarStart + 2);
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 2, 0);
ADD_MAIN_INSTRUCTION(IST_LessEq, exprStart + 5, exprStart + 3, exprStart + 4);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 12, exprStart + 5, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $n = $a . $b;
ADD_MAIN_INSTRUCTION(IST_Concat, exprStart + 0, localVarStart + 0, localVarStart + 1);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 13, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// end
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, -1, 0);
ADD_MAIN_INSTRUCTION(IST_Return, 0, 0, 0);

// run tests
TEST_RESULT("sources/expr.ifj", "expressions", 0, 0, ERR_None, 3, 0);

// Parser - function expr
// expected instructions
localVarStart = 2;
exprStart = localVarStart + 9;
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, exprStart, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, 1, 0);
// $a = noArg();
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Call, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 0, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $b = defArg1();
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_Call, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 1, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $c = defArg1($a + $b);
ADD_MAIN_INSTRUCTION(IST_Add, exprStart + 0, localVarStart + 0, localVarStart + 1);
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Push, 0, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_Call, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 2, exprStart + 1, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $d = defArg2($a . $b);
ADD_MAIN_INSTRUCTION(IST_Concat, exprStart + 0, localVarStart + 0, localVarStart + 1);
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Push, 0, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Call, 0, 2, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 3, exprStart + 1, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $e = defArg2($a - $b, $a * $b);
ADD_MAIN_INSTRUCTION(IST_Subtract, exprStart + 0, localVarStart + 0, localVarStart + 1);
ADD_MAIN_INSTRUCTION(IST_Multiply, exprStart + 1, localVarStart + 0, localVarStart + 1);
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Push, 0, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_Push, 0, exprStart + 1, 0);
ADD_MAIN_INSTRUCTION(IST_Call, 0, 2, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 4, exprStart + 2, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $f = defArg12();
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 3, 0);
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 2, 0);
ADD_MAIN_INSTRUCTION(IST_Call, 0, 3, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 5, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $g = defArg12($a + $b * $c);
ADD_MAIN_INSTRUCTION(IST_Multiply, exprStart + 0, localVarStart + 1, localVarStart + 2);
ADD_MAIN_INSTRUCTION(IST_Add, exprStart + 1, localVarStart + 0, exprStart + 0);
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Push, 0, exprStart + 1, 0);
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 2, 0);
ADD_MAIN_INSTRUCTION(IST_Call, 0, 3, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 6, exprStart + 2, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $h = defArg12(noArg(), $a !== $b);
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Call, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_NotEqual, exprStart + 1, localVarStart + 0, localVarStart + 1);
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Push, 0, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_Push, 0, exprStart + 1, 0);
ADD_MAIN_INSTRUCTION(IST_Call, 0, 3, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 7, exprStart + 2, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $i = defArg1(defArg1(defArg1()));
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_Call, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Push, 0, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_Call, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Push, 0, exprStart + 1, 0);
ADD_MAIN_INSTRUCTION(IST_Call, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 8, exprStart + 2, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// end
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, -1, 0);
ADD_MAIN_INSTRUCTION(IST_Return, 0, 0, 0);

// function instructions
// function noArg() { }
ADD_FUNC_INSTRUCTION(IST_Nullify, 0, -1, 0);
ADD_FUNC_INSTRUCTION(IST_Return, 0, 0, 0);
// function defArg1($a = 1) { }
ADD_FUNC_INSTRUCTION(IST_Nullify, 0, -2, 0);
ADD_FUNC_INSTRUCTION(IST_Return, 0, 1, 0);
// function defArg2($a, $b = 1) { }
ADD_FUNC_INSTRUCTION(IST_Nullify, 0, -3, 0);
ADD_FUNC_INSTRUCTION(IST_Return, 0, 2, 0);
// function defArg12($a = 1, $b = 2) { }
ADD_FUNC_INSTRUCTION(IST_Nullify, 0, -3, 0);
ADD_FUNC_INSTRUCTION(IST_Return, 0, 2, 0);

// run tests
TEST_RESULT("sources/func_expr.ifj", "function expr", 0, 0, ERR_None, 4, 4);

// Parser - precedence
// expected instructions
localVarStart = 2;
exprStart = localVarStart + 1;

// begin
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, exprStart, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, 1, 0);
// $x = 10;
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 0, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $x = true === $x < 20;
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 2, 0);
ADD_MAIN_INSTRUCTION(IST_Less, exprStart + 2, localVarStart + 0, exprStart + 1);
ADD_MAIN_INSTRUCTION(IST_Equal, exprStart + 3, exprStart + 0, exprStart + 2);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 0, exprStart + 3, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// end
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, -1, 0);
ADD_MAIN_INSTRUCTION(IST_Return, 0, 0, 0);

// run tests
TEST_RESULT("sources/relop_precedence.ifj", "rel. op precedence", 0, 0, ERR_None, 3, 0);

// Parser - if-elseif-else
// expected instructions
localVarStart = 2;
exprStart = localVarStart + 4;

// begin
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, exprStart, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, 1, 0);

// $a = 0
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 0, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);

// $b = 1.0
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 1, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);

// if ($a)
ADD_MAIN_INSTRUCTION(IST_Jmpz, exprStart, 2, localVarStart + 0);

// if ($a) end
ADD_MAIN_INSTRUCTION(IST_Jmp, 0, 10, 0);

// elseif ($b)
ADD_MAIN_INSTRUCTION(IST_Jmpz, exprStart, 5, localVarStart + 1);

// $c = true
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 2, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 2, exprStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);

// elseif ($b) end
ADD_MAIN_INSTRUCTION(IST_Jmp, 0, 5, 0);

// else
// $a = $a + 1
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 3, 0);
ADD_MAIN_INSTRUCTION(IST_Add, exprStart + 1, 2, exprStart + 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 0, exprStart + 1, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);

// if ($a)
ADD_MAIN_INSTRUCTION(IST_Jmpz, exprStart, 2, localVarStart + 0);

// if ($a) end
ADD_MAIN_INSTRUCTION(IST_Jmp, 0, 13, 0);

// elseif ($c)
ADD_MAIN_INSTRUCTION(IST_Jmpz, exprStart, 12, localVarStart + 2);

// $c = false
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 4, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 2, exprStart, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);

// if ($c)
ADD_MAIN_INSTRUCTION(IST_Jmpz, exprStart, 2, localVarStart + 2);

// if ($c) end
ADD_MAIN_INSTRUCTION(IST_Jmp, 0, 7, 0);

// elseif ($a)
ADD_MAIN_INSTRUCTION(IST_Jmpz, exprStart, 2, localVarStart + 0);

// elseif ($a) end
ADD_MAIN_INSTRUCTION(IST_Jmp, 0, 5, 0);

// else
// $a = $a + 2
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 5, 0);
ADD_MAIN_INSTRUCTION(IST_Add, exprStart + 1, localVarStart + 0, exprStart);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 0, exprStart + 1, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);

// $res = $a === 2
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 6, 0);
ADD_MAIN_INSTRUCTION(IST_Equal, exprStart + 1, localVarStart + 0, exprStart);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 3, exprStart + 1, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);

// end
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, -1, 0);
ADD_MAIN_INSTRUCTION(IST_Return, 0, 0, 0);

// run tests
TEST_RESULT("sources/if.ifj", "if-elseif-else", 0, 0, ERR_None, 7, 0);

// Parser - for statement
// expected instructions
localVarStart = 2;
exprStart = localVarStart + 3;

// begin
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, 5, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, 1, 0);
// $i = 0
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 0, 5, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// for
// for
// $i = $i + 1
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Add, 6, localVarStart + 0, 5);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 0, 6, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// if ($i < 10)
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 2, 0);
ADD_MAIN_INSTRUCTION(IST_Less, 6, localVarStart + 0, 5);
ADD_MAIN_INSTRUCTION(IST_Jmpz, exprStart, 2, 6);
// continue
ADD_MAIN_INSTRUCTION(IST_Jmp, 0, 2, 0);
// break
ADD_MAIN_INSTRUCTION(IST_Jmp, 0, 2, 0);
// for end
ADD_MAIN_INSTRUCTION(IST_Jmp, 0, -9, 0);
// break
ADD_MAIN_INSTRUCTION(IST_Jmp, 0, 2, 0);
// for end
ADD_MAIN_INSTRUCTION(IST_Jmp, 0, -11, 0);
// for
// $j = 0
ADD_MAIN_INSTRUCTION(IST_PushC, 0,   3, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 1, 5, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $j < $i
ADD_MAIN_INSTRUCTION(IST_Less, 5, localVarStart + 1, localVarStart + 0);
ADD_MAIN_INSTRUCTION(IST_Jmpz, exprStart, 13, 5);
// $x = put_string($j, "\n")
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 4, 0);
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Push, 0, 3, 0);
ADD_MAIN_INSTRUCTION(IST_Push, 0, 5, 0);
ADD_MAIN_INSTRUCTION(IST_PutString, 0, 2, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 2, 6, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $j = $j + 1
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 5, 0);
ADD_MAIN_INSTRUCTION(IST_Add, 6, localVarStart + 1, 5);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 1, 6, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// for end
ADD_MAIN_INSTRUCTION(IST_Jmp, 0, -13, 0);
// end
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, -1, 0);
ADD_MAIN_INSTRUCTION(IST_Return, 0, 0, 0);

// run tests
TEST_RESULT("sources/for.ifj", "for statement", 0, 0, ERR_None, 6, 0);

// Parser - while statement
// expected instructions
localVarStart = 2;
exprStart = localVarStart + 1;

// begin
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, 3, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, 1, 0);
// $i = 10
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 0, 3, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// while ($i)
ADD_MAIN_INSTRUCTION(IST_Jmpz, exprStart, 6, localVarStart + 0);
// $i = $i - 1
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Subtract, 4, localVarStart + 0, 3);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 0, 4, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// while end
ADD_MAIN_INSTRUCTION(IST_Jmp, 0, -5, 0);
// while ($i <= 10)
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 2, 0);
ADD_MAIN_INSTRUCTION(IST_LessEq, 4, localVarStart + 0, 3);
ADD_MAIN_INSTRUCTION(IST_Jmpz, exprStart, 8, 4);
// if (true)
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 3, 0);
ADD_MAIN_INSTRUCTION(IST_Jmpz, exprStart, 5, 3);
// $i = $i + 5
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 4, 0);
ADD_MAIN_INSTRUCTION(IST_Add, 4, localVarStart + 0, 3);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 0, 4, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// while end
ADD_MAIN_INSTRUCTION(IST_Jmp, 0, -9, 0);
// end
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, -1, 0);
ADD_MAIN_INSTRUCTION(IST_Return, 0, 0, 0);

TEST_RESULT("sources/while.ifj", "while statement", 0, 0, ERR_None, 5, 0);

// Parser - functions
// expected instructions
localVarStart = 2;
exprStart = localVarStart + 3;

// begin
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, 5, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, 1, 0);
// $a = 2
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 2, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 0, 5, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $x = 1
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 3, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 1, 5, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $res = dummy($x, $a)
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Push, 0, localVarStart + 1, 0);
ADD_MAIN_INSTRUCTION(IST_Push, 0, localVarStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_Call, 0, 2, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 2, 5, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// $res = one() + sqr($a) + sqr();
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Call, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Push, 0, localVarStart + 0, 0);
ADD_MAIN_INSTRUCTION(IST_Call, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Add, 7, 5, 6);
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_Call, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Add, 9, 7, 8);
ADD_MAIN_INSTRUCTION(IST_Mov, localVarStart + 2, 9, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// end
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, -1, 0);
ADD_MAIN_INSTRUCTION(IST_Return, 0, 0, 0);


// function one
exprStart = localVarStart + 0;
// begin
// return 1
ADD_FUNC_INSTRUCTION(IST_PushC, 0, 1, 0);
ADD_FUNC_INSTRUCTION(IST_Mov, -1, 2, 0);
ADD_FUNC_INSTRUCTION(IST_Return, 0, 0, 0);
// end
ADD_FUNC_INSTRUCTION(IST_Nullify, 0, -1, 0);
ADD_FUNC_INSTRUCTION(IST_Return, 0, 0, 0);

// function sqr
exprStart = localVarStart + 0;
// begin
// $x = $x * $x
ADD_FUNC_INSTRUCTION(IST_Multiply, 2, -1, -1);
ADD_FUNC_INSTRUCTION(IST_Mov, -1, 2, 0);
ADD_FUNC_INSTRUCTION(IST_ClearExpr, 0, exprStart, 0);
// return $x
ADD_FUNC_INSTRUCTION(IST_Mov, -2, -1, 0);
ADD_FUNC_INSTRUCTION(IST_Return, 0, 1, 0);
// end
ADD_FUNC_INSTRUCTION(IST_Nullify, 0, -2, 0);
ADD_FUNC_INSTRUCTION(IST_Return, 0, 1, 0);

// function dummy
exprStart = localVarStart + 0;
// begin
// end
ADD_FUNC_INSTRUCTION(IST_Nullify, 0, -3, 0);
ADD_FUNC_INSTRUCTION(IST_Return, 0, 2, 0);

TEST_RESULT("sources/function.ifj", "Functions", 0, 0, ERR_None, 4, 3);

// SAVE_STATE("sources/fun_nested.ifj", "sources/fun_nested.sta");

// HERE PUT ANOTHER TEST

/*************************************************************************************
************************       END OF TEST AREA            ***************************
*************************************************************************************/

freeInstructionVector(&expectedMainInstrVector);
freeInstructionVector(&expectedFuncInstrVector);

TEST_SUITE_END
