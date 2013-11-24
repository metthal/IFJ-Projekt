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

#define TEST_RESULT(testName, failed, funcFailed, errCode, constSize, addrSize) \
    if (tokenVector)                                                                    \
        parse(tokenVector, 1);                                                          \
    SHOULD_EQUAL("Parser - " testName " - error code", getError(), errCode);            \
    if (tokenVector) {                                                                  \
        testInstructions(mainInstructions, expectedMainInstrVector);                    \
        SHOULD_EQUAL("Parser - " testName " - instructions", instTestFailed, failed);   \
        testInstructions(functionsInstructions, expectedFuncInstrVector);               \
        SHOULD_EQUAL("Parser - " testName " - function instructions", instTestFailed, funcFailed);          \
        SHOULD_EQUAL("Parser - " testName " - const table size", vectorSize(constantsTable), constSize);    \
        SHOULD_EQUAL("Parser - " testName " - addr table size", vectorSize(addressTable), addrSize);        \
    }                                                                                                       \
    CLEANUP;

#define CLEANUP                             \
    clearError();                           \
    instTestFailed = 1;                     \
    vectorClearToken(expectedMainInstrVector);  \
    vectorClearToken(expectedFuncInstrVector);  \
    if (constantsTable)                         \
        freeValueVector(&constantsTable);       \
    if (addressTable)                           \
        freeInstructionPtrVector(&addressTable);\
    if (mainInstructions)                       \
        freeInstructionVector(&mainInstructions);   \
    if (functionsInstructions)                  \
        freeInstructionVector(&functionsInstructions);

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
            //printf("(Generated/Expected) (%d) %d(RES: %d, A: %d, B: %d) / %d(RES: %d, A: %d, B: %d)\n", count, itr1->code, itr1->res, itr1->a, itr1->b, itr2->code, itr2->res, itr2->a, itr2->b);
            allOk = 0;
        }
    }

    if (allOk)
        instTestFailed = 0;
}

void scanSourceFile(const char *filePath)
{
    tokenVector = scannerScanFile(filePath);
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
// input
scanSourceFile("sources/emptyfail.ifj");

// expected instructions
// none

// run tests
TEST_RESULT("missing PHP tag", 0, 0, ERR_Syntax, 0, 0);

// Parser - empty
// input
scanSourceFile("sources/empty.ifj");

// expected instructions
exprStart = 2;
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, exprStart, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, -1, 0);
ADD_MAIN_INSTRUCTION(IST_Return, 0, 0, 0);

// run tests
TEST_RESULT("empty", 0, 0, ERR_None, 0, 0);

// Parser - Assign
// input
scanSourceFile("sources/assign.ifj");

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
TEST_RESULT("assign", 0, 0, ERR_None, 1, 0);

// Parser - expressions
// input
scanSourceFile("sources/expr.ifj");

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
TEST_RESULT("expressions", 0, 0, ERR_None, 3, 0);

// HERE PUT ANOTHER TEST

/*************************************************************************************
************************       END OF TEST AREA            ***************************
*************************************************************************************/

freeInstructionVector(&expectedMainInstrVector);
freeInstructionVector(&expectedFuncInstrVector);


TEST_SUITE_END
