#include "parser.h"
#include "token_vector.h"
#include "instruction_vector.h"
#include "address_vector.h"
#include "ial.h"
#include "nierr.h"

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
uint32_t expr();

// Forward declaration of helper functions
uint32_t generalExpr(uint8_t skip);
uint32_t condition();
void stmtListBracketed();
void assignment(ConstTokenVectorIterator varid, uint8_t skip);
Symbol* addLocalVariable(ConstTokenVectorIterator varid);
Symbol* addParameter(ConstTokenVectorIterator varid, uint8_t defarg);

static const Vector *tokens = NULL;
// Safe to iterate without range checks because last and least
// one token will be EOF, therefore range is ensured implicitly
// by grammar rules.
static ConstTokenVectorIterator tokensIt = NULL;

SymbolTable *globalSymbolTable = NULL;

static Vector *addressTable = NULL;
static Vector *constantsTable = NULL;
static Vector *mainInstructions = NULL;
static Vector *functionsInstructions = NULL;
Vector *instructions = NULL;

static Context mainContext;
static Context *currentContext;

static uint8_t secondRun = 0;

// TODO Interpret creates stack frame for main
// returnValue
// stackPointer = NULL <- will know that it's program exit
// instructionPointer = NULL <- will know that it's program exit

// TODO !!!! Top is last item in vector - so that should be vectorBack?
// TODO Test all jumping - if, elseif, else, for, while, break, continue...
// TODO ptr1 in cycles might be called loop, ptr2 endJump

void parse(Vector* tokenVector)
{
    tokens = tokenVector;
    tokensIt = vectorBeginToken(tokenVector);

    globalSymbolTable = newSymbolTable();
    addressTable = newInstructionPtrVector();
    initContext(&mainContext);

    if (!getError()) {
        currentContext = &mainContext;
        secondRun = 0;
        prog();
    }

    mainInstructions = newInstructionVector();
    functionsInstructions = newInstructionVector();
    tokensIt = vectorBeginToken(tokenVector);

    if (!getError()) {
        currentContext = &mainContext;
        instructions = mainInstructions;
        secondRun = 1;
        prog();
    }

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

    tokensIt = NULL;
    instructions = NULL;
    currentContext = NULL;

    scannerReset();
    deleteContext(&mainContext);
    freeSymbolTable(&globalSymbolTable);
    freeTokenVector(&tokenVector);

    // TODO Move to interpreter cleanup
    freeInstructionPtrVector(&addressTable);
    freeInstructionVector(&mainInstructions);
    freeInstructionVector(&functionsInstructions);
}

void prog()
{
    switch (tokensIt->type) {
        case STT_Php:
            // Rule 1
            printf("%s\n", "Rule 1");
            tokensIt++;

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
            printf("%s\n", "Rule 4");
            break;

        case STT_Variable:
            // Rule 2
            printf("%s\n", "Rule 2");
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
                    printf("%s\n", "Rule 3");
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
                    printf("%s\n", "Rule 2");
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
        // Add new symbol with name of function to GST.
        symbol = symbolTableAdd(globalSymbolTable, &(tokensIt->str));
        if (getError())
            return;

        if (symbol == NULL) {
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
        // Reserve space for local variables
        generateInstruction(IST_Reserve, 0, currentContext->argumentCount, 0);
    }

    stmtListBracketed();
    if (getError())
        return;

    if (secondRun) {
        // Create instruction that will return null at the end of each function
        generateInstruction(IST_Nullify, 0, -(currentContext->argumentCount+1), 0);
        generateInstruction(IST_Return, 0, currentContext->argumentCount, 0);
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
            printf("%s\n", "Rule 6");
            break;

        case STT_Variable:
            // Rule 7
            printf("%s\n", "Rule 7");
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
                    printf("%s\n", "Rule 7");
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
            printf("%s\n", "Rule 8");

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
                    printf("%s\n", "Rule 9");
                    tokensIt++;

                    uint32_t exprRes = generalExpr(0);
                    if (getError())
                        return;

                    if (secondRun) {
                        // Move result of expression to reserved space for return value
                        generateInstruction(IST_Mov, -(currentContext->argumentCount+1), exprRes, 0);
                        generateInstruction(IST_Return, 0, currentContext->argumentCount, 0);
                    }

                    // Semicolon loaded by expr
                    if (tokensIt->type != STT_Semicolon) {
                        setError(ERR_Syntax);
                        return;
                    }

                    tokensIt++;

                    break;

                case KTT_Break:
                    // Rule 10
                    printf("%s\n", "Rule 10");
                    tokensIt++;

                    if (tokensIt->type != STT_Semicolon) {
                        setError(ERR_Syntax);
                        return;
                    }

                    if (secondRun) {
                        // TODO add to toBeModified Vector and
                        // reserve instruction
                    }

                    tokensIt++;

                    break;

                case KTT_Continue:
                    // Rule 11
                    printf("%s\n", "Rule 11");
                    tokensIt++;

                    if (tokensIt->type != STT_Semicolon) {
                        setError(ERR_Syntax);
                        return;
                    }

                    if (secondRun) {
                        // TODO add to toBeModified Vector and
                        // reserve instruction
                    }

                    tokensIt++;

                    break;

                case KTT_If: {
                    // Rule 12
                    printf("%s\n", "Rule 12");
                    tokensIt++;

                    uint32_t cond = condition();
                    if (getError())
                        return;

                    Instruction *ptr2 = NULL;
                    if (secondRun) {
                        // Reserves space for instruction that jumps
                        // to the end of this condition block
                        ptr2 = generateEmptyInstruction();
                    }

                    stmtListBracketed();
                    if (getError())
                        return;

                    uint32_t blockSize = (Instruction*)vectorBack(instructions) - ptr2;

                    blockSize += elseifStmt();
                    if (getError())
                        return;

                    if (secondRun) {
                        // Fills the reserved space with correct jump value
                        fillInstruction(ptr2, IST_Jmpz, 0, blockSize, cond);
                    }

                    break;
                }

                case KTT_While: {
                    // Rule 13 (almost same as Rule 12, missing just elseif call)
                    printf("%s\n", "Rule 13");
                    tokensIt++;

                    Instruction *ptr1 = NULL, *ptr2 = NULL;
                    ptr1 = (Instruction*)vectorBack(instructions);

                    uint32_t cond = condition();
                    if (getError())
                        return;

                    if (secondRun) {
                        // Reserves space for instruction that jumps
                        // behind while block
                        ptr2 = generateEmptyInstruction();
                    }

                    stmtListBracketed();
                    if (getError())
                        return;

                    if (secondRun) {
                        // Jump before condition for another iteration
                        generateInstruction(IST_Jmp, 0, ptr1 - (Instruction*)vectorBack(instructions), 0);

                        // Fills the reserved space with correct jump value
                        fillInstruction(ptr2, IST_Jmpz, 0, (Instruction*)vectorBack(instructions) - ptr2, cond);
                    }

                    break;
                }

                case KTT_For: {
                    // Rule 14
                    printf("%s\n", "Rule 14");
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

                    Instruction *ptr1 = NULL, *ptr2 = NULL, *ptr3 = NULL;

                    ptr1 = (Instruction*)vectorBack(instructions);

                    uint32_t cond;
                    // Check if 2nd statement weren't omitted.
                    if (secondRun && forStmt2(&cond)) {
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

                    stmtListBracketed();
                    if (getError())
                        return;

                    ptr3 = (Instruction*)vectorBack(instructions);

                    // Generate For's 3rd statement
                    if (secondRun) {
                        ConstTokenVectorIterator afterForBlock = tokensIt;
                        tokensIt = beforeFor3;
                        forStmt1(0);
                        if (getError())
                            return;
                        tokensIt = afterForBlock;

                        // Jump before condition for another iteration
                        generateInstruction(IST_Jmp, 0, ptr1 - (Instruction*)vectorBack(instructions), 0);

                        if (ptr2 != NULL) {
                            // Fills the reserved space with correct jump value
                            fillInstruction(ptr2, IST_Jmpz, 0, (Instruction*)vectorBack(instructions) - ptr2, cond);
                        }

                        // Finish everything by filling pre-generated
                        // break and and continue instructions
                        uint16_t fillCount = 10; // number of break or continue stmts
                        for (uint16_t i = 0; i < fillCount; i++) {
                            // TODO
                            /*// topPtr is top in toBeModified Vector
                            if (continuestmt) {
                                // topPtr = INSTR JMP [ptr3 - topPtr]
                            }
                            else if (breakstmt) {
                                // topPtr = INSTR JMP [Top - topPtr]
                            }

                            pop;*/
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
            printf("%s\n", "Rule 15");

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
                    printf("%s\n", "Rule 15");

                    return elseStmt();

                case KTT_Elseif:
                    // Rule 16 (almost same as Rule 12)
                    printf("%s\n", "Rule 16");
                    tokensIt++;

                    Instruction *ptr1 = NULL, *ptr2 = NULL;

                    if (secondRun) {
                        // Reserves space for instruction that jumps
                        // to the end of whole if-else block
                        ptr1 = generateEmptyInstruction();
                    }

                    uint32_t cond = condition();
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

                    uint32_t blockSize = (Instruction*)vectorBack(instructions) - ptr2;

                    blockSize += elseifStmt();
                    if (getError())
                        return 0;

                    if (secondRun) {
                        // Fills the reserved space with correct jump value
                        fillInstruction(ptr1, IST_Jmp, 0, (Instruction*)vectorBack(instructions) - ptr1, 0);

                        // Fills the reserved space with correct jump value
                        fillInstruction(ptr1, IST_Jmpz, 0, blockSize, cond);
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
            printf("%s\n", "Rule 17");
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
                    printf("%s\n", "Rule 17");
                    break;

                case KTT_Else:
                    // Rule 18
                    printf("%s\n", "Rule 18");

                    Instruction *ptr1 = NULL;
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
                        fillInstruction(ptr1, IST_Jmp, 0, (Instruction*)vectorBack(instructions) - ptr1, 0);
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

void paramList()
{
    switch (tokensIt->type) {
        case STT_RightBracket:
            // Rule 19
            printf("%s\n", "Rule 19");
            break;

        case STT_Variable:
            // Rule 20
            printf("%s\n", "Rule 20");

            ConstTokenVectorIterator varid = tokensIt;
            uint8_t defarg = 0;

            tokensIt++;
            // Test for default argument
            if (tokensIt->type == STT_Assignment) {
                tokensIt++;
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

        default:
            setError(ERR_Syntax);
    }
}

void nparamList(uint8_t defarg)
{
    switch (tokensIt->type) {
        case STT_RightBracket:
            // Rule 21
            printf("%s\n", "Rule 21");
            break;

        case STT_Comma:
            // Rule 22
            printf("%s\n", "Rule 22");
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
                tokensIt++;
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
            printf("%s\n", "Rule 23");
            break;

        case STT_Variable:
            // Rule 24 (almost same as the rule 8, missing just semicolon)
            printf("%s\n", "Rule 24");

            ConstTokenVectorIterator varid = tokensIt;

            tokensIt++;

            assignment(varid, skip);
            if (getError())
                return;

            break;

        default:
            setError(ERR_Syntax);
    }
}

uint8_t forStmt2(uint32_t *cond)
{
    switch (tokensIt->type) {
        case STT_Semicolon:
            // Rule 25
            printf("%s\n", "Rule 25");
            break;

        default:
            printf("%s\n", "Rule 26");

            *cond = generalExpr(0);
            if(getError())
                break;

            return 1;
    }

    return 0;
}

// @TODO implement bottom up parser for expression
uint32_t expr()
{
    int leftBrackets = 0;
    while (1) {
        switch (tokensIt->type) {
            case STT_Semicolon:
            case STT_EOF:
                return 0;

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
    return 0;
}

uint32_t generalExpr(uint8_t skip)
{
    if (secondRun && !skip) {
        return expr();
    }
    else {
        // Skips expression.
        // If error occured, it won't change token iterator.
        ConstTokenVectorIterator backup = tokensIt;
        int leftBrackets = 0;
        while (1) {
            switch (tokensIt->type) {
                case STT_Semicolon:
                    return 0;

                case STT_Assignment:
                case STT_Keyword:
                case STT_LeftCurlyBracket:
                case STT_RightCurlyBracket:
                case STT_Php:
                case STT_EOF:
                    setError(ERR_Syntax);
                    tokensIt = backup;
                    // TODO Make full syntax check to discover true error?
                    // expr();
                    return 0;

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

uint32_t condition()
{
    if (tokensIt->type != STT_LeftBracket) {
        setError(ERR_Syntax);
        return 0;
    }

    tokensIt++;

    uint32_t exprRes = generalExpr(0);
    if (getError())
        return 0;

    // Right bracket loaded by expr
    if (tokensIt->type != STT_RightBracket) {
        setError(ERR_Syntax);
        return 0;
    }

    tokensIt++;
    return exprRes;
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

    // TODO Variable can become constant here
    uint32_t exprRes = generalExpr(skip);

    if (secondRun) {
        // Symbol should be already in table after first run
        Symbol *symbol = symbolTableFind(currentContext->localTable, &(varid->str));
        if (getError())
           return;

        // Variable is declared after assignment and can be used
        symbol->data->var.declared = 1;

        // Move result of expression to variable
        generateInstruction(IST_Mov, symbol->data->var.relativeIndex, exprRes, 0);
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
    // Space reserved at stack frame for Stack and Instruction Pointers
    uint32_t reserved = 2;

    Symbol *symbol = symbolTableAdd(currentContext->localTable, &(varid->str));
    if (getError())
        return NULL;

    if (symbol == NULL)
        symbol = symbolTableFind(currentContext->localTable, &(varid->str));
    else {
        symbol->type = ST_Variable;
        symbol->data = (SymbolData*)newVariableSymbolData();
        symbol->data->var.relativeIndex = reserved + currentContext->localVariableCount;
        symbol->data->var.declared = 0;
        symbol->data->var.constantIndex = 0;
        symbol->data->var.constant = 0;
        currentContext->localVariableCount++;
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
    symbol->data = (SymbolData*)newVariableSymbolData();
    currentContext->argumentCount++;
    symbol->data->var.relativeIndex = -currentContext->argumentCount;

    symbol->data->var.declared = 1;


    if (defarg) {
        // TODO check if varid+2 is literal or null
        // Add it to constant table and set symbol->data->var.constantIndex
        symbol->data->var.constant = 1;
        // Otherwise set this error
        setError(ERR_BadDefArg);
        return NULL;
    }
    else {
        symbol->data->var.constantIndex = 0;
        symbol->data->var.constant = 0;
    }

    return symbol;
}
