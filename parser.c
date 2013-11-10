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
void nparamList();
void forStmt1();
uint32_t forStmt2();
uint32_t expr();

// Forward declaration of helper functions
uint32_t condition();
void stmtListBracketed();
void assignment(ConstTokenVectorIterator varid);
Symbol* addLocalVariable(ConstTokenVectorIterator varid);
Symbol* addParameter(ConstTokenVectorIterator varid);

static const Vector *tokens = NULL;
// Safe to iterate without range checks because last and least
// one token will be EOF, therefore range is ensured implicitly
// by grammar rules.
static ConstTokenVectorIterator tokensIt = NULL;

static SymbolTable *globalSymbolTable = NULL;

static Vector *addressTable = NULL;
static Vector *constantsTable = NULL;
static Vector *mainInstructions = NULL;
static Vector *functionsInstructions = NULL;
static Vector *instructions = NULL;

static Context mainContext;
static Context currentContext;

static uint8_t secondRun = 0;

// TODO !!!! Top is last item in vector
// TODO Test all jumping - if, elseif, else, for, while, break, continue...
// TODO ptr1 in cycles might be called loop, ptr2 endJump
// TODO INSTR JMP [ptr1 - Top] can be read as jump to ptr1 from Top
// TODO work with second run - rework everything except func (check that one).

void parse(Vector* tokenVector)
{
    tokens = tokenVector;
    tokensIt = vectorBeginToken(tokenVector);

    globalSymbolTable = newSymbolTable();
    addressTable = newInstructionPtrVector();
    initContext(&mainContext);

    if (!getError()) {
        currentContext = mainContext;
        secondRun = 0;
        prog();
    }

    mainInstructions = newInstructionVector();
    functionsInstructions = newInstructionVector();

    if (!getError()) {
        currentContext = mainContext;
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

    deleteContext(&mainContext);
    freeSymbolTable(&globalSymbolTable);
    freeTokenVector(&tokenVector);
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

        // TODO if symbol already in GST, syntax error.

        // Set symbol type.
        symbol->type = ST_Function;
        // Reserve space in address table for future InstructionPtr
        vectorPushDefaultInstructionPtr(addressTable);
        // Set relative index to space reserved above.
        symbol->data->func.functionAddressIndex = vectorSize(addressTable) - 1;
    }
    else {
        // Symbol should be already in the table so just find it.
        symbol = symbolTableFind(globalSymbolTable, &tokensIt->str);
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
    // working with it's SymbolTable and localc.
    currentContext = symbol->data->func.context;

    if (!secondRun) {
        // Load arguments to LST in first run.
        // TODO actually load them.
        paramList();
    }

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
        // TODO add instruction that reserves space for local variables
    }

    // TODO generate instructions
    stmtListBracketed();
    if (getError())
        return;

    if (secondRun) {
        // TODO add instruction in form of "return null"
        instructions = mainInstructions;
    }

    // Switch context back to that of main.
    currentContext = mainContext;
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

            assignment(varid);
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

                    // INSTR MOV RETURN_LOC, expr-result
                    expr();
                    if (getError())
                        return;

                    // TODO Create stack frame for main
                    // returnValue
                    // Vector* address for stack <-- Stack pointer
                    // HALT instruction - return from interpret with returnValue

                    // INSTR RETURN paramCount (from context)

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

                    // TODO add to toBeModified Vector and
                    // reserve instruction

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

                    // TODO add to toBeModified Vector and
                    // reserve instruction

                    tokensIt++;

                    break;

                case KTT_If: {
                    // Rule 12
                    printf("%s\n", "Rule 12");
                    tokensIt++;

                    uint32_t cond = condition();
                    if (getError())
                        return;

                    // Reserves space for instruction that jumps
                    // to the end of this condition block
                    // INSTR DEFAULT (save pointer = ptr2)

                    stmtListBracketed();
                    if (getError())
                        return;

                    // uint32_t blockSize = Top - ptr2 + 1

                    // blockSize += elseifStmt();
                    if (getError())
                        return;

                    // Fills the reserved space with correct jump value
                    // ptr2 = INSTR JMPZ cond, blockSize

                    break;
                }

                case KTT_While: {
                    // Rule 13 (almost same as Rule 12, missing just elseif call)
                    printf("%s\n", "Rule 13");
                    tokensIt++;

                    // TODO ptr1 = Top

                    uint32_t cond = condition();
                    if (getError())
                        return;

                    // Reserves space for instruction that jumps
                    // behind while block
                    // INSTR DEFAULT (save pointer = ptr2)

                    stmtListBracketed();
                    if (getError())
                        return;

                    // Jump before condition for another iteration
                    // INSTR JMP [ptr1 - Top + 1]

                    // Fills the reserved space with correct jump value
                    // ptr2 = INSTR JMPZ cond, [Top - ptr2 + 1]

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

                    forStmt1();
                    if (getError())
                        return;

                    // Semicolon loaded by forStmt1
                    if (tokensIt->type != STT_Semicolon) {
                        setError(ERR_Syntax);
                        return;
                    }

                    tokensIt++;

                    // TODO ptr1 = Top

                    uint32_t cond = forStmt2();
                    if (getError())
                        return;

                    // Check if 2nd statement weren't omitted. Condition
                    // result can't be at 0, as there's program return value.
                    if (cond != 0) {
                        // Reserves space for instruction that jumps
                        // behind for block
                        // INSTR DEFAULT (save pointer = ptr2)
                    }

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
                    // Skip everything until brace ending for header.
                    { // Leave garbage in scope to prevent stack overflow.
                        uint8_t error = 0;
                        int braceCounter = 1;
                        while (!error && braceCounter > 0) {
                            tokensIt++;

                            switch (tokensIt->type) {
                                // Optimization to stop searching on error
                                // These can't be part of 3rd statement
                                case STT_LeftCurlyBracket:
                                case STT_Semicolon:
                                case STT_EOF:
                                    error = 1;
                                    break;

                                case STT_RightBracket:
                                    braceCounter--;
                                    break;
                                case STT_LeftBracket:
                                    braceCounter++;
                                    break;

                                default:
                                    break;
                            }
                        }

                        // On error, re-run with full syntax check
                        // to get correct error report
                        if (error) {
                            tokensIt = beforeFor3;
                            forStmt1();
                            if (getError())
                                return;

                            // Right bracket loaded by forStmt1
                            if (tokensIt->type != STT_RightBracket) {
                                setError(ERR_Syntax);
                                return;
                            }
                        }
                    }

                    // Right bracket checked during skipping
                    // Load left curly bracket
                    tokensIt++;

                    stmtListBracketed();
                    if (getError())
                        return;

                    // TODO repeat = Top

                    // Generate For's 3rd statement
                    ConstTokenVectorIterator afterForBlock = tokensIt;
                    tokensIt = beforeFor3;
                    forStmt1();
                    if (getError())
                        return;
                    tokensIt = afterForBlock;

                    // Jump before condition for another iteration
                    // INSTR JMP [ptr1 - Top + 1]

                    if (cond != 0) {
                        // Fills the reserved space with correct jump value
                        // ptr2 = INSTR JMPZ cond, [Top - ptr2 + 1]
                    }

                    // Finish everything by filling pre-generated
                    // break and and continue instructions
                    uint16_t fillCount = 10; // number of break or continue stmts
                    for (uint16_t i = 0; i < fillCount; i++) {
                        /*// topPtr is top in toBeModified Vector
                        if (continuestmt) {
                            // topPtr = INSTR JMP [repeat - topPtr + 1]
                        }
                        else if (breakstmt) {
                            // topPtr = INSTR JMP [Top - topPtr + 1]
                        }

                        pop;*/
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

            uint8_t ret = elseStmt();
            if (getError())
                return 0;

            return ret;

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

                    uint8_t ret = elseStmt();
                    if (getError())
                        return 0;

                    return ret;

                case KTT_Elseif:
                    // Rule 16 (almost same as Rule 12)
                    printf("%s\n", "Rule 16");
                    tokensIt++;

                    // Reserves space for instruction that jumps
                    // to the end of whole if-else block
                    // INSTR DEFAULT (save pointer = ptr1)

                    uint32_t cond = condition();
                    if (getError())
                        return 0;

                    // Reserves space for instruction that jumps
                    // to the end of this condition block
                    // INSTR DEFAULT (save pointer = ptr2)

                    stmtListBracketed();
                    if (getError())
                        return 0;

                    // uint32_t blockSize = Top - ptr2 + 1

                    // blockSize += elseifStmt();
                    if (getError())
                        return 0;

                    // Fills the reserved space with correct jump value
                    // ptr1 = INSTR JMP [Top - ptr1 + 1]

                    // Fills the reserved space with correct jump value
                    // ptr2 = INSTR JMPZ cond, blockSize

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

                    // Reserves space for instruction that jumps
                    // to the end of whole if-else block
                    // INSTR DEFAULT (save pointer = ptr1)

                    tokensIt++;

                    stmtListBracketed();
                    if (getError())
                        return 0;

                    // Fills the reserved space with correct jump value
                    // ptr1 = INSTR JMP [Top - ptr1 + 1]

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

            if (getError())
                return;

            ConstTokenVectorIterator token = tokensIt;

            nparamList();
            if (getError())
                return;

            addParameter(token);
            if (getError())
               return;

            break;

        default:
            setError(ERR_Syntax);
    }
}

void nparamList()
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

            ConstTokenVectorIterator token = tokensIt;

            nparamList();
            if (getError())
                return;

            addParameter(token);
            if (getError())
               return;

            break;

        default:
            setError(ERR_Syntax);
    }
}

void forStmt1()
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

            assignment(varid);
            if (getError())
                return;

            break;

        default:
            setError(ERR_Syntax);
    }
}

uint32_t forStmt2()
{
    switch (tokensIt->type) {
        case STT_Semicolon:
            // Rule 25
            printf("%s\n", "Rule 25");
            break;

        default:
            // @TODO Tokens for top down parsing doesn't have
            // to be send as it will be certainly syntax error.
            // Need predict(EXPR) to determine which.
            printf("%s\n", "Rule 26");
            uint32_t exprRes = expr();
            if (getError())
                return 0;

            return exprRes;
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

uint32_t condition()
{
    if (tokensIt->type != STT_LeftBracket) {
        setError(ERR_Syntax);
        return 0;
    }

    tokensIt++;

    uint32_t exprRes = expr();
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

void assignment(ConstTokenVectorIterator varid)
{
    if (tokensIt->type != STT_Assignment) {
        setError(ERR_Syntax);
        return;
    }

    tokensIt++;

    Symbol *symbol = addLocalVariable(varid);
    if (getError())
       return;

    uint32_t exprRes = expr();
    if (getError())
        return;

    // INSTR MOV symbol->data->relativeIndex, exprRes
}

// Adds local variable to symbol table.
// If already present, returns the existing symbol.
Symbol* addLocalVariable(ConstTokenVectorIterator varid)
{
    // Space reserved at stack frame for Stack and Instruction Pointers
    uint32_t reserved = 2;

    Symbol *symbol = symbolTableAdd(currentContext.localTable, &(varid->str));
    if (getError())
        return NULL;

    if (symbol == NULL)
        symbol = symbolTableFind(currentContext.localTable, &(varid->str));
    else {
        symbol->type = ST_Variable;
        //symbol->data = newVariableSymbolData();
        currentContext.localVariableCount++;
        symbol->data->var.relativeIndex = reserved + currentContext.localVariableCount;
        // TODO default value
    }

    return symbol;
}

Symbol* addParameter(ConstTokenVectorIterator varid)
{
    Symbol *symbol = symbolTableAdd(currentContext.localTable, &(varid->str));
    if (getError())
        return NULL;

    symbol->type = ST_Variable;
    //symbol->data = newVariableSymbolData();
    currentContext.argumentCount++;
    symbol->data->var.relativeIndex = -currentContext.argumentCount;
    // TODO default value

    return symbol;
}
