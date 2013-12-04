#include "parser.h"
#include "builtin.h"
#include "interpreter.h"
#include "token_vector.h"
#include "uint32_vector.h"
#include "instruction_vector.h"
#include "address_vector.h"
#include "value_vector.h"
#include "ial.h"
#include "nierr.h"
#include "expr.h"

#include <stdlib.h>
#include <stdint.h>

// Forward declaration of recursive
// nonterminal functions
void prog();
void body();
void func();
void stmtList();
void stmt();
uint8_t elseifStmt();
uint8_t elseStmt();
void paramList();
void nparamList(uint8_t defarg);
void forStmt1(uint8_t skip);
uint8_t forStmt2(uint32_t *cond);

// Forward declaration of helper functions
int64_t generalExpr(uint8_t skip, int64_t resultOffset, uint32_t *maxStackPosUsed);
int64_t condition(uint8_t skip);
void stmtListBracketed();
void assignment(ConstTokenVectorIterator varid, uint8_t skip);
Symbol* addLocalVariable(ConstTokenVectorIterator varid);
Symbol* addParameter(ConstTokenVectorIterator varid, uint8_t defarg);

static const Vector *tokens = NULL;
// Safe to iterate without range checks because last and least
// one token will be EOF, therefore range is ensured implicitly
// by grammar rules.
ConstTokenVectorIterator tokensIt = NULL;

SymbolTable *globalSymbolTable = NULL;

Vector *instructions = NULL;
Vector *constantsTable = NULL;
Vector *addressTable = NULL;
Vector *mainInstructions = NULL;
Vector *functionsInstructions = NULL;
static Vector *toBeModifiedIST = NULL;

static Context mainContext;
Context *currentContext;

// Space reserved at stack frame for Stack and Instruction Pointers
static const uint32_t stackFrameReserved = 2;

static uint8_t secondRun = 0;
static uint8_t cycleScope = 0;

// TODO Test all jumping - if, elseif, else, for, while, break, continue...
// TODO ptr1 in cycles might be called loop, ptr2 endJump

void parse(Vector *tokenVector, uint8_t testRun)
{
    tokens = tokenVector;
    tokensIt = vectorBeginToken(tokenVector);

    globalSymbolTable = newSymbolTable();
    constantsTable = newValueVector();
    addressTable = newInstructionPtrVector();
    initContext(&mainContext);

    if (!getError()) {
        // Initialize exprStart in case of no local variables
        mainContext.exprStart = stackFrameReserved;
        currentContext = &mainContext;
        secondRun = 0;
        prog();
    }

    mainInstructions = newInstructionVector();
    functionsInstructions = newInstructionVector();
    toBeModifiedIST = newUint32Vector();
    tokensIt = vectorBeginToken(tokenVector);
    initExpr();

    if (!getError()) {
        currentContext = &mainContext;
        instructions = mainInstructions;
        secondRun = 1;
        prog();
    }

    tokensIt = NULL;
    instructions = NULL;
    currentContext = NULL;

    deinitExpr();
    deleteContext(&mainContext);
    freeSymbolTable(&globalSymbolTable);
    freeTokenVector(&tokenVector);
    freeUint32Vector(&toBeModifiedIST);

    // Shrink vectors to save space
    vectorShrinkToFit(constantsTable);
    vectorShrinkToFit(addressTable);
    vectorShrinkToFit(mainInstructions);
    vectorShrinkToFit(functionsInstructions);

    // Here we can end, save necessary structures and interpret later

    if (!getError()) {
        // Recalculate items in address table to real addresses
        Instruction* firstReal = vectorBeginInstruction(functionsInstructions);
        for (InstructionPtrVectorIterator it = vectorBeginInstructionPtr(
                addressTable); it != vectorEndInstructionPtr(addressTable);
                it++) {
            // Items in address table were previously
            // filled with relative index to function's
            // first instruction inside functionsInstructions.
            (*it) = firstReal + (size_t)(*it);
        }
    }

    if (!testRun) {
        if(!getError())
            interpret(vectorBeginInstruction(mainInstructions), constantsTable, addressTable);

        // Cleanup after interpretation
        freeValueVector(&constantsTable);
        freeInstructionPtrVector(&addressTable);
        freeInstructionVector(&mainInstructions);
        freeInstructionVector(&functionsInstructions);
    }
}

void prog()
{
    switch (tokensIt->type) {
        case STT_Php:
            // Rule 1
            tokensIt++;

            if (secondRun) {
                // First instruction should reserve space on stack
                generateInstruction(IST_Reserve, ISM_NoConst, 0, currentContext->exprStart, 0);
                // Nullify pointers to mark program end
                generateInstruction(IST_Nullify, ISM_NoConst, 0, 0, 0);
                generateInstruction(IST_Nullify, ISM_NoConst, 0, 1, 0);
            }

            body();
            if (getError())
                return;
            break;
        default:
            setError(ERR_Syntax);
    }
}

void body()
{
    switch (tokensIt->type) {
        case STT_EOF:
            // Rule 4
            if (secondRun) {
                generateInstruction(IST_Nullify, ISM_NoConst, 0, -1, 0);
                generateInstruction(IST_Return, ISM_NoConst, 0, 0, 0);
            }
            break;

        case STT_Variable:
            // Rule 2
            stmt();
            if (getError())
                return;

            body();
            if (getError())
                return;

            break;

        case STT_Keyword:
            switch (tokensIt->keywordType) {
                case KTT_Function:
                    // Rule 3
                    func();
                    if (getError())
                        return;

                    body();
                    if (getError())
                        return;

                    break;

                case KTT_Return:
                case KTT_Break:
                case KTT_Continue:
                case KTT_If:
                case KTT_While:
                case KTT_For:
                    // Rule 2
                    stmt();
                    if (getError())
                        return;

                    body();
                    if (getError())
                        return;

                    break;

                default:
                    setError(ERR_Syntax);
            }
            break;

        default:
            setError(ERR_Syntax);
    }
}

void func()
{
    // Callable only from Rule 3 with terminal. Function
    // doesn't need switch.
    tokensIt++;

    if (tokensIt->type != STT_Identifier) {
        setError(ERR_Syntax);
        return;
    }

    Symbol *symbol;
    if (!secondRun) {
        // Test if function's name doesn't collide with builtin's.
        if (getBuiltinCode(&(tokensIt->str)) != BTI_None) {
            setError(ERR_RedefFunction);
            return;
        }

        // Add new symbol with name of function to GST.
        symbol = symbolTableAdd(globalSymbolTable, &(tokensIt->str));
        if (getError())
            return;

        if (symbol == NULL) {
            // Function's name already exists in symbol table.
            setError(ERR_RedefFunction);
            return;
        }

        // Set symbol type.
        symbol->type = ST_Function;
        symbol->data = (SymbolData*)newFunction();
        // Reserve space in address table for future InstructionPtr
        vectorPushDefaultInstructionPtr(addressTable);
        // Set relative index to space reserved above.
        symbol->data->func.functionAddressIndex = vectorSize(addressTable) - 1;
    }
    else {
        // Symbol should be already in the table so just find it.
        symbol = symbolTableFind(globalSymbolTable, &(tokensIt->str));
    }

    if (getError())
        return;

    tokensIt++;

    if (tokensIt->type != STT_LeftBracket) {
        setError(ERR_Syntax);
        return;
    }

    tokensIt++;

    // Switch context to that of function, so all rules will be
    // working with it's SymbolTable and localVariableCount.
    currentContext = &symbol->data->func.context;

    paramList();
    if (getError())
        return;

    // Right bracket loaded by paramList.
    if (tokensIt->type != STT_RightBracket) {
        setError(ERR_Syntax);
        return;
    }

    tokensIt++;

    if (secondRun) {
        // Switch place to where instructions for functions are generated
        instructions = functionsInstructions;
        // Set address in address table to point to index of first function's instruction
        InstructionPtr* cipvi =
                vectorAt(addressTable, symbol->data->func.functionAddressIndex);
        (*cipvi) = (InstructionPtr)((size_t)vectorSize(functionsInstructions));

        // Instruction to reserve stack space for locals
        if (currentContext->localVariableCount > 0)
            generateInstruction(IST_Reserve, ISM_NoConst, 0, currentContext->localVariableCount, 0);
    }
    else {
        // Initialize exprStart in case of no local variables
        currentContext->exprStart = stackFrameReserved;
    }

    stmtListBracketed();
    if (getError())
        return;

    if (secondRun) {
        // Create instruction that will return null at the end of each function
        generateInstruction(IST_Nullify, ISM_NoConst, 0, -(currentContext->argumentCount+1), 0);
        generateInstruction(IST_Return, ISM_NoConst, 0, currentContext->argumentCount, 0);
        instructions = mainInstructions;
    }

    // Switch context back to that of main.
    currentContext = &mainContext;
}

void stmtList()
{
    switch (tokensIt->type) {
        case STT_RightCurlyBracket:
            // Rule 6
            break;

        case STT_Variable:
            // Rule 7
            stmt();
            if (getError())
                return;

            stmtList();
            if (getError())
                return;

            break;

        case STT_Keyword:
            switch (tokensIt->keywordType) {
                case KTT_Return:
                case KTT_Break:
                case KTT_Continue:
                case KTT_If:
                case KTT_While:
                case KTT_For:
                    // Rule 7
                    stmt();
                    if (getError())
                        return;

                    stmtList();
                    if (getError())
                        return;

                    break;

                default:
                    setError(ERR_Syntax);
            }
            break;

        default:
            setError(ERR_Syntax);
    }
}

void stmt()
{
    switch (tokensIt->type) {
        case STT_Variable: {
            // Rule 8
            ConstTokenVectorIterator varid = tokensIt;

            tokensIt++;

            assignment(varid, 0);
            if (getError())
                return;

            // Semicolon loaded by assignment
            if (tokensIt->type != STT_Semicolon) {
                setError(ERR_Syntax);
                return;
            }

            tokensIt++;

            break;
        }

        case STT_Keyword:
            switch (tokensIt->keywordType) {
                case KTT_Return:
                    // Rule 9
                    tokensIt++;

                    generalExpr(0, -(currentContext->argumentCount + 1), NULL);
                    if (getError())
                        return;

                    if (secondRun)
                        generateInstruction(IST_Return, ISM_NoConst, 0, currentContext->argumentCount, 0);

                    // Semicolon loaded by expr
                    if (tokensIt->type != STT_Semicolon) {
                        setError(ERR_Syntax);
                        return;
                    }

                    tokensIt++;

                    break;

                case KTT_Break:
                    // Rule 10
                    tokensIt++;

                    if (!cycleScope) {
                        setError(ERR_CycleControl);
                        return;
                    }

                    if (tokensIt->type != STT_Semicolon) {
                        setError(ERR_Syntax);
                        return;
                    }

                    if (secondRun) {
                        vectorPushUint32(toBeModifiedIST, vectorSize(instructions));
                        generateInstruction(IST_Break, ISM_NoConst, 0, 0, 0);
                    }

                    tokensIt++;

                    break;

                case KTT_Continue:
                    // Rule 11
                    tokensIt++;

                    if (!cycleScope) {
                        setError(ERR_CycleControl);
                        return;
                    }

                    if (tokensIt->type != STT_Semicolon) {
                        setError(ERR_Syntax);
                        return;
                    }

                    if (secondRun) {
                        vectorPushUint32(toBeModifiedIST, vectorSize(instructions));
                        generateInstruction(IST_Continue, ISM_NoConst, 0, 0, 0);
                    }

                    tokensIt++;

                    break;

                case KTT_If: {
                    // Rule 12
                    tokensIt++;

                    int64_t cond = condition(0);
                    if (getError())
                        return;

                    uint32_t ptr2 = 0;
                    if (secondRun) {
                        // Reserves space for instruction that jumps
                        // to the end of this condition block
                        ptr2 = generateEmptyInstruction();
                    }

                    stmtListBracketed();
                    if (getError())
                        return;

                    uint32_t blockSize = 0;
                    if (secondRun)
                        blockSize = vectorSize(instructions) - ptr2;

                    blockSize += elseifStmt();
                    if (getError())
                        return;

                    if (secondRun) {
                        // Fills the reserved space with correct jump value
                        fillInstruction(ptr2, IST_Jmpz, currentContext->exprStart, blockSize, cond);
                    }

                    break;
                }

                case KTT_While: {
                    // Rule 13 (almost same as Rule 12, missing just elseif call)
                    tokensIt++;

                    uint32_t ptr1 = 0;
                    if (secondRun) {
                        // Reserves space for instruction that jumps
                        // to while's condition
                        ptr1 = generateEmptyInstruction();
                    }

                    ConstTokenVectorIterator beforeCond = tokensIt;

                    condition(1);
                    if (getError())
                        return;

                    stmtListBracketed();
                    if (getError())
                        return;

                    ConstTokenVectorIterator afterWhileBlock = tokensIt;

                    if (secondRun) {
                        // Fills the reserved space with correct jump value
                        fillInstruction(ptr1, IST_Jmp, 0, vectorSize(instructions) - ptr1, 0);

                        tokensIt = beforeCond;
                        int64_t cond = condition(0);
                        tokensIt = afterWhileBlock;

                        // Conditional jump to beginning of while cycle.
                        generateInstruction(IST_Jmpnz, ISM_NoConst, currentContext->exprStart, ptr1 + 1 - vectorSize(instructions), cond);
                    }

                    break;
                }

                case KTT_For: {
                    // Rule 14
                    tokensIt++;

                    if (tokensIt->type != STT_LeftBracket) {
                        setError(ERR_Syntax);
                        return;
                    }

                    tokensIt++;

                    forStmt1(0);
                    if (getError())
                        return;

                    // Semicolon loaded by forStmt1
                    if (tokensIt->type != STT_Semicolon) {
                        setError(ERR_Syntax);
                        return;
                    }

                    tokensIt++;

                    // Break / continue counter
                    uint32_t bcCounter = 0;
                    uint32_t ptr1 = 0, ptr2 = 0, ptr3 = 0;

                    if (secondRun) {
                        ptr1 = vectorSize(instructions);
                        bcCounter = vectorSize(toBeModifiedIST);
                    }

                    uint32_t cond;
                    uint8_t for2Used = forStmt2(&cond);
                    // Check if 2nd statement weren't omitted.
                    if (secondRun && for2Used) {
                        // Reserves space for instruction that jumps
                        // behind for block
                        ptr2 = generateEmptyInstruction();
                    }
                    if (getError())
                        return;

                    // Semicolon loaded by forStmt2
                    if (tokensIt->type != STT_Semicolon) {
                        setError(ERR_Syntax);
                        return;
                    }

                    tokensIt++;

                    // A hack to move 3rd for statement after
                    // statement list to minimize number of
                    // jumps during interpretation.
                    // Save token pointer to return here later.
                    ConstTokenVectorIterator beforeFor3 = tokensIt;
                    forStmt1(1);
                    if (getError())
                        return;

                    // Right bracket loaded by forStmt1
                    if (tokensIt->type != STT_RightBracket) {
                        setError(ERR_Syntax);
                        return;
                    }

                    tokensIt++;

                    {
                        // Set cycle scope if not set
                        uint8_t highestCycle = 0;
                        if (!cycleScope)
                            cycleScope = highestCycle = 1;

                        stmtListBracketed();
                        if (getError())
                            return;

                        // Unset cycle scope if set by this statement
                        if (highestCycle)
                            cycleScope = 0;
                    }

                    // Generate For's 3rd statement
                    if (secondRun) {
                        ptr3 = vectorSize(instructions);
                        ConstTokenVectorIterator afterForBlock = tokensIt;
                        tokensIt = beforeFor3;
                        forStmt1(0);
                        if (getError())
                            return;
                        tokensIt = afterForBlock;

                        // Jump before condition for another iteration
                        generateInstruction(IST_Jmp, ISM_NoConst, 0, ptr1 - vectorSize(instructions), 0);

                        if (for2Used) {
                            // Fills the reserved space with correct jump value
                            fillInstruction(ptr2, IST_Jmpz, currentContext->exprStart, vectorSize(instructions) - ptr2, cond);
                        }

                        // Finish everything by filling pre-generated
                        // break and and continue instructions
                        uint32_t tbmSize = vectorSize(toBeModifiedIST);
                        for (; bcCounter < tbmSize; bcCounter++) {
                            Uint32 index = *((Uint32*)vectorBack(toBeModifiedIST));
                            Instruction* pt = vectorAt(instructions, index);
                            if (pt->code == IST_Continue)
                                fillInstruction(index, IST_Jmp, 0, ptr3 - index, 0);
                            else if (pt->code == IST_Break)
                                fillInstruction(index, IST_Jmp, 0, vectorSize(instructions) - index, 0);

                            vectorPopUint32(toBeModifiedIST);
                        }
                    }

                    break;
                }

                default:
                    setError(ERR_Syntax);
            }
            break;

        default:
            setError(ERR_Syntax);
    }
}

// Returns 1 if Rule 16
uint8_t elseifStmt()
{
    switch (tokensIt->type) {
        case STT_EOF:
        case STT_Variable:
        case STT_RightCurlyBracket:
            // Rule 15
            return elseStmt();

        case STT_Keyword:
            switch (tokensIt->keywordType) {
                case KTT_Function:
                case KTT_Return:
                case KTT_Break:
                case KTT_Continue:
                case KTT_If:
                case KTT_Else:
                case KTT_While:
                case KTT_For:
                    // Rule 15
                    return elseStmt();

                case KTT_Elseif:
                    // Rule 16 (almost same as Rule 12)
                    tokensIt++;

                    uint32_t ptr1 = 0, ptr2 = 0;

                    if (secondRun) {
                        // Reserves space for instruction that jumps
                        // to the end of whole if-else block
                        ptr1 = generateEmptyInstruction();
                    }

                    int64_t cond = condition(0);
                    if (getError())
                        return 0;

                    if (secondRun) {
                        // Reserves space for instruction that jumps
                        // to the end of this condition block
                        ptr2 = generateEmptyInstruction();
                    }

                    stmtListBracketed();
                    if (getError())
                        return 0;

                    uint32_t blockSize = 0;
                    if (secondRun)
                        blockSize = vectorSize(instructions) - ptr2;

                    blockSize += elseifStmt();
                    if (getError())
                        return 0;

                    if (secondRun) {
                        // Fills the reserved space with correct jump value
                        fillInstruction(ptr1, IST_Jmp, 0, vectorSize(instructions) - ptr1, 0);

                        // Fills the reserved space with correct jump value
                        fillInstruction(ptr2, IST_Jmpz, currentContext->exprStart, blockSize, cond);
                    }

                    return 1;

                default:
                    setError(ERR_Syntax);
            }
            break;

        default:
            setError(ERR_Syntax);
    }

    return 0;
}

// Returns 1 if Rule 18
uint8_t elseStmt()
{
    switch (tokensIt->type) {
        case STT_EOF:
        case STT_Variable:
        case STT_RightCurlyBracket:
            // Rule 17
            break;

        case STT_Keyword:
            switch (tokensIt->keywordType) {
                case KTT_Function:
                case KTT_Return:
                case KTT_Break:
                case KTT_Continue:
                case KTT_If:
                case KTT_While:
                case KTT_For:
                    // Rule 17
                    break;

                case KTT_Else: {
                    // Rule 18
                    uint32_t ptr1 = 0;
                    if (secondRun) {
                        // Reserves space for instruction that jumps
                        // to the end of whole if-else block
                        ptr1 = generateEmptyInstruction();
                    }

                    tokensIt++;

                    stmtListBracketed();
                    if (getError())
                        return 0;

                    if (secondRun) {
                        // Fills the reserved space with correct jump value
                        fillInstruction(ptr1, IST_Jmp, 0, vectorSize(instructions) - ptr1, 0);
                    }

                    return 1;
                }
                default:
                    setError(ERR_Syntax);
            }
            break;

        default:
            setError(ERR_Syntax);
    }
    return 0;
}

void paramList()
{
    switch (tokensIt->type) {
        case STT_RightBracket:
            // Rule 19
            break;

        case STT_Variable: {
            // Rule 20
            ConstTokenVectorIterator varid = tokensIt;
            uint8_t defarg = 0;

            tokensIt++;
            // Test for default argument
            if (tokensIt->type == STT_Assignment) {
                tokensIt += 2; // skip also default value handled in addParameter
                defarg = 1;
            }

            nparamList(defarg);
            if (getError())
                return;

            if (!secondRun) {
                addParameter(varid, defarg);
                if (getError())
                   return;
            }

            break;
        }
        default:
            setError(ERR_Syntax);
    }
}

void nparamList(uint8_t defarg)
{
    switch (tokensIt->type) {
        case STT_RightBracket:
            // Rule 21
            break;

        case STT_Comma:
            // Rule 22
            tokensIt++;

            if (tokensIt->type != STT_Variable) {
                setError(ERR_Syntax);
                return;
            }

            if (getError())
                return;

            ConstTokenVectorIterator varid = tokensIt;

            tokensIt++;
            // Test for default argument
            if (tokensIt->type == STT_Assignment) {
                tokensIt += 2; // skip also default value handled in addParameter
                defarg = 1;
            }
            else if (defarg) {
                setError(ERR_DefArgOrder);
                return;
            }

            nparamList(defarg);
            if (getError())
                return;

            if (!secondRun) {
                addParameter(varid, defarg);
                if (getError())
                   return;
            }

            break;

        default:
            setError(ERR_Syntax);
    }
}

void forStmt1(uint8_t skip)
{
    switch (tokensIt->type) {
        case STT_RightBracket:
        case STT_Semicolon:
            // Rule 23
            break;

        case STT_Variable: {
            // Rule 24 (almost same as the rule 8, missing just semicolon)
            ConstTokenVectorIterator varid = tokensIt;

            tokensIt++;

            assignment(varid, skip);
            if (getError())
                return;

            break;
        }
        default:
            setError(ERR_Syntax);
    }
}

uint8_t forStmt2(uint32_t *cond)
{
    switch (tokensIt->type) {
        case STT_Semicolon:
            // Rule 25
            break;

        default:
            // Rule 26
            *cond = generalExpr(0, 0, NULL);
            if(getError())
                break;

            return 1;
    }

    return 0;
}

int64_t generalExpr(uint8_t skip, int64_t resultOffset, uint32_t *maxStackPosUsed)
{
    if (secondRun && !skip) {
        return expr(resultOffset, maxStackPosUsed);
    }
    else {
        // Skips expression.
        // TODO decide if needed (for full expression check)
        // ConstTokenVectorIterator backup = tokensIt;
        int leftBrackets = 0;
        while (1) {
            switch (tokensIt->type) {
                case STT_Semicolon:
                    if (leftBrackets > 0) {
                        setError(ERR_Syntax);
                        // tokensIt = backup;
                    }
                    return 0;

                case STT_Assignment:
                case STT_LeftCurlyBracket:
                case STT_RightCurlyBracket:
                case STT_Php:
                case STT_EOF:
                    setError(ERR_Syntax);
                    // tokensIt = backup;
                    return 0;

                case STT_Keyword:
                    switch (tokensIt->keywordType) {
                        // Keywords that can be part of expression
                        // explicitly allowed

                        // Default is error
                        default:
                            setError(ERR_Syntax);
                            // tokensIt = backup;
                            return 0;
                    }
                    break;

                case STT_LeftBracket:
                    leftBrackets++;
                    break;

                case STT_RightBracket:
                    if (leftBrackets == 0)
                        return 0;
                    leftBrackets--;
                    break;

                default:
                    break;
            }

            tokensIt++;
        }
    }
    return 0;
}

int64_t condition(uint8_t skip)
{
    if (tokensIt->type != STT_LeftBracket) {
        setError(ERR_Syntax);
        return 0;
    }

    tokensIt++;

    // Use return value as temporary place for result of condition
    int64_t exprRes = generalExpr(skip, -currentContext->argumentCount - 1, NULL);
    if (getError())
        return 0;

    // Right bracket loaded by expr
    if (tokensIt->type != STT_RightBracket) {
        setError(ERR_Syntax);
        return 0;
    }

    tokensIt++;
    //TODO why exprRes is giving wrong index?
    return -currentContext->argumentCount - 1;
}

void stmtListBracketed()
{
    if (tokensIt->type != STT_LeftCurlyBracket) {
        setError(ERR_Syntax);
        return;
    }

    tokensIt++;

    stmtList();
    if (getError())
        return;

    // Right curly bracket loaded by stmtList
    if (tokensIt->type != STT_RightCurlyBracket) {
        setError(ERR_Syntax);
        return;
    }

    tokensIt++;
}

void assignment(ConstTokenVectorIterator varid, uint8_t skip)
{
    if (tokensIt->type != STT_Assignment) {
        setError(ERR_Syntax);
        return;
    }

    tokensIt++;

    uint32_t maxStackPosUsed = currentContext->exprStart;
    if (secondRun && !skip) {
        // Symbol should be already in table after first run
        Symbol *symbol = symbolTableFind(currentContext->localTable, &(varid->str));
        if (getError())
           return;

        generalExpr(skip, symbol->data->var.relativeIndex, &maxStackPosUsed);
    }
    else
        generalExpr(skip, 0, NULL);

    if (getError())
        return;

    if (secondRun && !skip && maxStackPosUsed > currentContext->exprStart) {
        // Clear generated expression space
        generateInstruction(IST_ClearExpr, ISM_NoConst, 0, currentContext->exprStart, 0);
    }
    else {
        addLocalVariable(varid);
        if (getError())
           return;
    }
}

// Adds local variable to symbol table.
// If already present, returns the existing symbol.
Symbol* addLocalVariable(ConstTokenVectorIterator varid)
{
    Symbol *symbol = symbolTableAdd(currentContext->localTable, &(varid->str));
    if (getError())
        return NULL;

    if (symbol == NULL)
        symbol = symbolTableFind(currentContext->localTable, &(varid->str));
    else {
        symbol->type = ST_Variable;
        symbol->data = (SymbolData*)newVariable();
        symbol->data->var.relativeIndex = stackFrameReserved + currentContext->localVariableCount;
        currentContext->localVariableCount++;
        currentContext->exprStart = stackFrameReserved + currentContext->localVariableCount;
    }

    return symbol;
}

// Adds parameter to symbol table.
// If already present, sets error.
Symbol* addParameter(ConstTokenVectorIterator varid, uint8_t defarg)
{
    Symbol *symbol = symbolTableAdd(currentContext->localTable, &(varid->str));
    if (getError())
        return NULL;

    if (symbol == NULL) {
        setError(ERR_RedefParameter);
        return NULL;
    }

    symbol->type = ST_Variable;
    symbol->data = (SymbolData*)newVariable();
    currentContext->argumentCount++;
    symbol->data->var.relativeIndex = -currentContext->argumentCount;

    if (defarg) {
        // Add varid+2 to constant table if literal, else set error
        vectorPushDefaultValue(constantsTable);
        tokenToValue(varid+2, vectorBack(constantsTable));

        if(getError()) {
            // Convert error for better description
            setError(ERR_BadDefArg);
            return NULL;
        }

        if (currentContext->defaultCount == 0) {
            // Remember first defarg index to constant table
            currentContext->defaultStart = vectorSize(constantsTable) - 1;
        }
        currentContext->defaultCount++;
    }

    return symbol;
}
