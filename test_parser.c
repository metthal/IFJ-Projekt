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

    for ( ; (itr1 != end1) && (itr2 != end2); ++itr1, ++itr2) {
        if (!INSTRUCTIONS_EQ(itr1, itr2)) {
            // COMMENT OUT THIS LINE IF YOU WANT TO COMPARE INSTRUCTIONS
            //printf("(Generated/Expected) %d(RES: %d, A: %d, B: %d) / %d(RES: %d, A: %d, B: %d)\n", itr1->code, itr1->res, itr1->a, itr1->b, itr2->code, itr2->res, itr2->a, itr2->b);
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
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, 2, 0);
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
ADD_MAIN_INSTRUCTION(IST_Reserve, 0, 4, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, 1, 0);
ADD_MAIN_INSTRUCTION(IST_PushC, 0, 0, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, 2, 4, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, 4, 0);
ADD_MAIN_INSTRUCTION(IST_Mov, 3, 2, 0);
ADD_MAIN_INSTRUCTION(IST_ClearExpr, 0, 4, 0);
ADD_MAIN_INSTRUCTION(IST_Nullify, 0, -1, 0);
ADD_MAIN_INSTRUCTION(IST_Return, 0, 0, 0);

// run tests
TEST_RESULT("assign", 0, 0, ERR_None, 1, 0);

// HERE PUT ANOTHER TEST

/*************************************************************************************
************************       END OF TEST AREA            ***************************
*************************************************************************************/

freeInstructionVector(&expectedMainInstrVector);
freeInstructionVector(&expectedFuncInstrVector);


TEST_SUITE_END
