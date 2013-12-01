/**
 * @file expr.c
 * @brief Bottom-up parser for expressions
 **/
#include "expr.h"
#include "nierr.h"
#include "token_vector.h"
#include "context.h"
#include "ial.h"
#include "instruction.h"
#include "value_vector.h"

// definitions from parser which need to be present also in expression parser
extern ConstTokenVectorIterator tokensIt;
extern Context *currentContext;
extern Vector *constantsTable;
extern Vector *instructions;

static Vector *exprVector = NULL;   ///< Bottom-up parser stack for @ref ExprVector
static ExprToken endToken;          ///< Token that should be located on the bottom of the stack, used when stack has no topmost terminal token
static uint32_t currentStackPos;    ///< Current position in the local stack frame
static uint32_t lastResultInstIndex;
static uint32_t lastFuncParamCount;

// used as helper variables for function call generation
static BuiltinCode currentFuncBuiltinCode;  ///< BuiltinCode for currently processed function
static int64_t currentFuncParamLimit;       ///< Max. parameters pushed on stack for currently processed function
static Symbol *currentFuncSymbol;           ///< Symbol in SymbolTable for currently processed function

/**
 * Defines type of the token located on the bottom-up parser stack
 **/
typedef enum
{
    Terminal,   ///< Terminal token
    NonTerminal ///< Non-terminal token
} ExprTokenType;

/**
 * Defines priority between tokens on the stack and input token
 **/
typedef enum
{
    Low,        ///< Token on the top of the stack has lower priority than input token
    High,       ///< Token on the top of the stack has higher priority than input token
    Equal,      ///< Token on the top of the stack has same priority as input token
    Error       ///< Token on the top of the stack cannot be followed by input token, syntax error
} TokenPrecedence;

/**
 * Precedence table defines all combinations of top of the stack/input token and priorities between them.
 * Rows mean top of the stack and columns mean input token.
 **/
static uint8_t precedenceTable[STT_Semicolon][STT_Semicolon] =
{
//      +       -       *       /       .       <       >       <=      >=      ===     !==     &&      ||      !       (       )       func    ,       $       var
    {   High,   High,   Low,    Low,    Low,    High,   High,   High,   High,   High,   High,   High,   High,   Low,    Low,    High,   Low,    High,   High,   Low   },  // +
    {   High,   High,   Low,    Low,    Low,    High,   High,   High,   High,   High,   High,   High,   High,   Low,    Low,    High,   Low,    High,   High,   Low   },  // -
    {   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   Low,    Low,    High,   Low,    High,   High,   Low   },  // *
    {   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   Low,    Low,    High,   Low,    High,   High,   Low   },  // /
    {   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   Low,    Low,    High,   Low,    High,   High,   Low   },  // .
    {   Low,    Low,    Low,    Low,    Low,    High,   High,   High,   High,   High,   High,   High,   High,   Low,    Low,    High,   Low,    High,   High,   Low   },  // <
    {   Low,    Low,    Low,    Low,    Low,    High,   High,   High,   High,   High,   High,   High,   High,   Low,    Low,    High,   Low,    High,   High,   Low   },  // >
    {   Low,    Low,    Low,    Low,    Low,    High,   High,   High,   High,   High,   High,   High,   High,   Low,    Low,    High,   Low,    High,   High,   Low   },  // <=
    {   Low,    Low,    Low,    Low,    Low,    High,   High,   High,   High,   High,   High,   High,   High,   Low,    Low,    High,   Low,    High,   High,   Low   },  // >=
    {   Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    High,   High,   High,   High,   Low,    Low,    High,   Low,    High,   High,   Low   },  // ===
    {   Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    High,   High,   High,   High,   Low,    Low,    High,   Low,    High,   High,   Low   },  // !==
    {   Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    High,   High,   Low,    Low,    High,   Low,    High,   High,   Low   },  // &&
    {   Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    High,   Low,    Low,    High,   Low,    High,   High,   Low   },  // ||
    {   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   Low,    Low,    High,   Low,    High,   High,   Low   },  // !
    {   Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Equal,  Low,    Equal,  Error,  Low   },  // (
    {   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   Error,  High,   Error,  High,   High,   Error },  // )
    {   Error,  Error,  Error,  Error,  Error,  Error,  Error,  Error,  Error,  Error,  Error,  Error,  Error,  Error,  Equal,  Error,  Error,  Error,  Error,  Error },  // func
    {   Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Equal,  Low,    Equal,  Error,  Low   },  // ,
    {   Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Error,  Low,    Error,  Error,  Low   },  // $
    {   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   Error,  Error,  High,   Error,  High,   High,   Error }   // var
};

/**
 * Initializes bottom-up parser.
 **/
void initExpr()
{
    exprVector = newExprTokenVector();
    endToken.token = newToken();
    endToken.token->type = STT_EOF;
    endToken.type = Terminal;
}


/**
 * Performs cleanup after bottom-up parser.
 **/
void deinitExpr()
{
    free(endToken.token);
    vectorClearExprToken(exprVector);
    freeExprTokenVector(&exprVector);
}

/**
 * Traverses through the stack and finds the token which is terminal and is closest to the top of the stack
 *
 * @return Topmost terminal token. NULL if not found
 **/
ExprToken* findTopmostTerminal()
{
    ExprTokenVectorIterator itr = vectorEndExprToken(exprVector);

    while (itr != vectorBeginExprToken(exprVector)) {
        itr--;
        if (itr->type == Terminal)
            return itr;
    }

    return NULL;
}

/**
 * Transforms token type into index in precedence table.
 *
 * @param tokenType Token type to transform
 *
 * @return Index in precedence table
 **/
static inline uint8_t tokenTypeToExprType(uint8_t tokenType)
{
    if (tokenType >= STT_Semicolon)
        return STT_EOF;

    if (tokenType >= STT_Variable && tokenType <= STT_String)
        return STT_Variable;

    return tokenType;
}

/**
 * Transforms token type into the instruction code.
 *
 * @param tokenType Token type to transform
 *
 * @return Instruction code for the token type
 **/
static inline uint8_t tokenTypeToInstruction(uint8_t tokenType)
{
    switch (tokenType) {
        case STT_Plus:
            return IST_Add;

        case STT_Minus:
            return IST_Subtract;

        case STT_Multiply:
            return IST_Multiply;

        case STT_Divide:
            return IST_Divide;

        case STT_Dot:
            return IST_Concat;

        case STT_Equal:
            return IST_Equal;

        case STT_NotEqual:
            return IST_NotEqual;

        case STT_Less:
            return IST_Less;

        case STT_LessEqual:
            return IST_LessEq;

        case STT_Greater:
            return IST_Greater;

        case STT_GreaterEqual:
            return IST_GreaterEq;

        case STT_And:
            return IST_And;

        case STT_Or:
            return IST_Or;

        default:
            return IST_Noop;
    }
}

static inline void modifyExprInstResult(uint32_t instIndex, uint32_t resultOffset)
{
    Instruction *inst = vectorAt(instructions, instIndex);

    switch (inst->code) {
        case IST_PushC:
            inst->code = IST_MovC;
            inst->res = resultOffset;
            break;
        case IST_Reserve:
            inst->code = IST_PushRef;
            inst->a = -(lastFuncParamCount + currentStackPos) + resultOffset;
            break;
        case IST_Add:
        case IST_Subtract:
        case IST_Multiply:
        case IST_Divide:
        case IST_Concat:
        case IST_Equal:
        case IST_NotEqual:
        case IST_Less:
        case IST_LessEq:
        case IST_Greater:
        case IST_GreaterEq:
        case IST_Or:
            inst->res = resultOffset;
            break;
        default:
            setError(ERR_Internal);
            break;
    }
}

/**
 * Reduces expression for multiparameter functions recursively.
 *
 * @param stackPos Position on the stack where last parameter of the function is located
 * @param paramCount Set to the number of parameters, cannot be NULL
 *
 * @return Returns 1 in case of success, otherwise 0
 **/
uint8_t reduceMultiparamFunc(uint32_t stackPos, uint32_t *paramCount, uint32_t *totalParamCount)
{
    if (stackPos == 0 || paramCount == NULL)
        return 0;

    ExprTokenVectorIterator first = vectorAt(exprVector, stackPos);
    ExprTokenVectorIterator second = first - 1;
    if (first->type == NonTerminal) { // got E,E..) expect , or (
        if (second->type == Terminal && second->token->type == STT_Comma) { // we got ,E,E..), enter recursion
            if (!reduceMultiparamFunc(stackPos - 2, paramCount, totalParamCount))
                return 0;
        }
        else if (second->type == Terminal && second->token->type == STT_LeftBracket) { // got (E,..,E), expect id
            if (stackPos < 2) {
                setError(ERR_Syntax);
                return 0;
            }

            // check if we have id token before (
            ExprTokenVectorIterator id = first - 2;
            if (id->type != Terminal || (id->type == Terminal && id->token->type != STT_Identifier)) {
                setError(ERR_Syntax);
                return 0;
            }

            currentFuncSymbol = fillInstFuncInfo(id->token, &currentFuncBuiltinCode, &currentFuncParamLimit);
            if (getError())
                return 0;
        }
        else {
            setError(ERR_Syntax);
            return 0;
        }
    }
    else {
        setError(ERR_Syntax);
        return 0;
    }

    // if it doesn't matter on parameter count or we reached the limit
    if (currentFuncParamLimit == -1 || *paramCount < currentFuncParamLimit) {
        generateInstruction(IST_Push, 0, first->stackOffset, 0);
        if (getError())
            return 0;

        (*paramCount)++;
    }
    (*totalParamCount)++;

    return 1;
}

/**
 * Performs reduction by rule in grammar
 *
 * @param topTerm Topmost terminal token on the stack
 *
 * @return Returns 1 in case of succes, otherwise 0
 **/
uint8_t reduce(ExprToken *topTerm)
{
    switch (topTerm->token->type) {
        case STT_Number:
        case STT_Double:
        case STT_Null:
        case STT_Bool:
        case STT_String:
            vectorPushDefaultValue(constantsTable);
            tokenToValue(topTerm->token, vectorBack(constantsTable));
            if (getError())
                return 0;

            generateInstruction(IST_PushC, 0, vectorSize(constantsTable) - 1, 0);
            if (getError())
                return 0;

            topTerm->type = NonTerminal;
            topTerm->stackOffset = currentStackPos++;
            lastResultInstIndex = vectorSize(instructions) - 1;
            break;
        case STT_Variable: {
            Symbol *symbol = symbolTableFind(currentContext->localTable, &(topTerm->token->str));
            if (symbol == NULL) {
                setError(ERR_UndefVariable);
                return 0;
            }

            topTerm->type = NonTerminal;
            topTerm->stackOffset = symbol->data->var.relativeIndex;
            lastResultInstIndex = 1;
            break;
        }
        case STT_Not: {
            uint32_t stackSize = vectorSize(exprVector);
            if (stackSize < 2) {
                setError(ERR_Syntax);
                return 0;
            }

            ExprTokenVectorIterator operand = vectorBack(exprVector);
            if (operand->type != NonTerminal) {
                setError(ERR_Syntax);
                return 0;
            }

            ExprTokenVectorIterator notOperator = operand - 1;
            if (notOperator != topTerm) {
                setError(ERR_Syntax);
                return 0;
            }

            uint32_t notCount = 1;
            if (stackSize > 2) {
                int64_t stackPos = stackSize - 3;

                // get the number of ! operators so we can perform optimalization
                // for odd count perform one negation
                // for even count just convert to bool
                do
                {
                    notOperator--;
                    if (notOperator->type != Terminal || (notOperator->type == Terminal && notOperator->token->type != STT_Not)) {
                        notOperator++; // restore the last token that was !
                        break;
                    }

                    stackPos--;
                } while (stackPos != -1);

                notCount += (stackSize - 3 - stackPos);
            }

            notOperator->type = NonTerminal;
            notOperator->stackOffset = currentStackPos++;
            generateInstruction(IST_Not, notOperator->stackOffset, operand->stackOffset, notCount & 1); // same as notCount % 2
            if (getError())
                return 0;

            vectorPopNExprToken(exprVector, notCount);
            break;
        }
        case STT_And:
        case STT_Or:
        case STT_Equal:
        case STT_NotEqual:
        case STT_Less:
        case STT_Greater:
        case STT_LessEqual:
        case STT_GreaterEqual:
        case STT_Minus:
        case STT_Multiply:
        case STT_Divide:
        case STT_Dot:
        case STT_Plus: {
            uint32_t stackSize = vectorSize(exprVector);
            if (stackSize < 3) {
                setError(ERR_Syntax);
                return 0;
            }

            ExprTokenVectorIterator operand2 = vectorBack(exprVector);
            if (operand2->type != NonTerminal) {
                setError(ERR_Syntax);
                return 0;
            }

            ExprTokenVectorIterator operator = operand2 - 1;
            if (operator != topTerm) {
                setError(ERR_Syntax);
                return 0;
            }

            ExprTokenVectorIterator operand1 = operand2 - 2;
            if (operand1->type != NonTerminal) {
                setError(ERR_Syntax);
                return 0;
            }

            generateInstruction(tokenTypeToInstruction(operator->token->type), currentStackPos, operand1->stackOffset, operand2->stackOffset);
            if (getError())
                return 0;

            lastResultInstIndex = vectorSize(instructions) - 1;
            operand1->stackOffset = currentStackPos++;
            vectorPopNExprToken(exprVector, 2);
            break;
        }
        case STT_RightBracket: {
            uint32_t stackSize = vectorSize(exprVector);
            if (stackSize < 3) {
                setError(ERR_Syntax);
                return 0;
            }

            ExprTokenVectorIterator nextTerm = vectorBack(exprVector);
            if (nextTerm == topTerm) { // at top must be )
                nextTerm--;

                if (nextTerm->type == Terminal && nextTerm->token->type == STT_LeftBracket) { // it's function if there is ()
                    nextTerm--;

                    if (nextTerm->type == Terminal && nextTerm->token->type == STT_Identifier) {
                        uint32_t retValStackPos = currentStackPos++;
                        generateInstruction(IST_Reserve, 0, 1, 0);
                        if (getError())
                            return 0;

                        lastResultInstIndex = vectorSize(instructions) - 1;
                        currentFuncSymbol = fillInstFuncInfo(nextTerm->token, &currentFuncBuiltinCode, &currentFuncParamLimit);
                        if (getError())
                            return 0;

                        lastFuncParamCount = generateCall(currentFuncSymbol, currentFuncBuiltinCode, 0);
                        if (getError())
                            return 0;

                        nextTerm->type = NonTerminal;
                        nextTerm->stackOffset = retValStackPos;
                        vectorPopNExprToken(exprVector, 2);
                    }
                    else {
                        setError(ERR_Syntax);
                        return 0;
                    }
                }
                else if (nextTerm->type == NonTerminal) { // we have E) and are not sure what it is
                    ExprTokenVectorIterator exprBackup = nextTerm;
                    nextTerm--;

                    if (nextTerm->type == Terminal && nextTerm->token->type == STT_Comma) { // we have ,E) and it's function
                        uint32_t stackPos = stackSize - 2;

                        if (stackPos < 4) { // not enough space for best case func(E,E)
                            setError(ERR_Syntax);
                            return 0;
                        }

                        uint32_t retValStackPos = currentStackPos++;
                        generateInstruction(IST_Reserve, 0, 1, 0);
                        if (getError())
                            return 0;

                        lastResultInstIndex = vectorSize(instructions) - 1;
                        uint32_t paramCount = 0, totalParamCount = 0;
                        if (!reduceMultiparamFunc(stackPos, &paramCount, &totalParamCount))
                            return 0;

                        vectorPopNExprToken(exprVector, (totalParamCount << 1) + 1); // paramCount * 2 + 1 (every E has one terminal in front of it and there is one ending ')')
                        nextTerm = vectorBack(exprVector);
                        if (nextTerm->type != Terminal || (nextTerm->type == Terminal && nextTerm->token->type != STT_Identifier)) {
                            setError(ERR_Syntax);
                            return 0;
                        }

                        lastFuncParamCount = generateCall(currentFuncSymbol, currentFuncBuiltinCode, paramCount);
                        if (getError())
                            return 0;

                        nextTerm->type = NonTerminal;
                        nextTerm->stackOffset = retValStackPos;
                    }
                    else if (nextTerm->type == Terminal && nextTerm->token->type == STT_LeftBracket) { // it's (E) and we don't know if it is single param func or just (E)
                        if (stackSize >= 4) {
                            ExprTokenVectorIterator leftBracketBackup = nextTerm; // we need to backup current token for (E) case
                            nextTerm--;

                            if (nextTerm->type == Terminal && nextTerm->token->type == STT_Identifier) { // we have id(E) and it's function
                                // return value
                                uint32_t retValStackPos = currentStackPos++;
                                generateInstruction(IST_Reserve, 0, 1, 0);
                                if (getError())
                                    return 0;

                                lastResultInstIndex = vectorSize(instructions) - 1;
                                currentFuncSymbol = fillInstFuncInfo(nextTerm->token, &currentFuncBuiltinCode, &currentFuncParamLimit);
                                if (getError())
                                    return 0;

                                // if there is no parameter limit or it is higher than 0
                                if (currentFuncParamLimit == -1 || currentFuncParamLimit > 0) {
                                    generateInstruction(IST_Push, 0, exprBackup->stackOffset, 0);
                                    if (getError())
                                        return 0;
                                }

                                lastFuncParamCount = generateCall(currentFuncSymbol, currentFuncBuiltinCode, currentFuncParamLimit == 0 ? 0 : 1);
                                if (getError())
                                    return 0;

                                nextTerm->type = NonTerminal;
                                nextTerm->stackOffset = retValStackPos;
                                vectorPopNExprToken(exprVector, 3);
                            }
                            else { // it's just (E)
                                leftBracketBackup->type = NonTerminal;
                                leftBracketBackup->stackOffset = exprBackup->stackOffset;
                                vectorPopNExprToken(exprVector, 2);
                            }
                        }
                        else { // it can only be (E) if we have just 3 items on the stack
                            nextTerm->type = NonTerminal;
                            nextTerm->stackOffset = exprBackup->stackOffset;
                            vectorPopNExprToken(exprVector, 2);
                        }
                    }
                    else {
                        setError(ERR_Syntax);
                        return 0;
                    }
                }
                else {
                    setError(ERR_Syntax);
                    return 0;
                }
            }
            else {
                setError(ERR_Syntax);
                return 0;
            }
            break;
        }
        default:
            return 0;
    }

    return 1;
}

/**
 * Initializes new token for the bottom-up parser stack.
 *
 * @param token Token to initialize
 **/
void initExprToken(ExprToken *token)
{
    memset(token, 0, sizeof(ExprToken));
}

/**
 * Deinitializes token used in the bottom-up parser stack.
 *
 * @param token Token to deinitialize
 **/
void deleteExprToken(ExprToken *token)
{
    (void)token;
}

/**
 * Copies one expression token into the other
 *
 * @param src Source expression token
 * @param dest Destination expression token
 **/
void copyExprToken(const ExprToken *src, ExprToken *dest)
{
    dest->type = src->type;
    dest->token = src->token;
}


/**
 * Based on the input token and its priority to the token on the top of the stack performs shift or reduce.
 *
 * @return Offset in the local stack frame where the result is stored, in case of error 0
 **/
uint32_t expr(uint32_t resultOffset)
{
    // insert bottom of the stack (special kind of token) to the stack
    uint8_t endOfInput = 0;
    const Token *currentToken = NULL;
    vectorClearExprToken(exprVector);
    currentStackPos = currentContext->exprStart;
    lastResultInstIndex = 0;

    while (1) {
        if (!endOfInput) {
            if (tokensIt->type < STT_Semicolon) // invalid token received
                currentToken = tokensIt;
            else {
                currentToken = endToken.token;
                endOfInput = 1;
            }
        }

        ExprToken *topToken = findTopmostTerminal();

        // no terminal found
        if (!topToken) {
            // there is just one non-terminal on the stack, we succeeded
            // OR
            // if there is no topmost terminal and input is right bracket, end successfuly
            // top-down parser will take care of bad input, since he will assume I loaded first token after expression
            if ((endOfInput || currentToken->type == STT_RightBracket) && vectorSize(exprVector) == 1) {
                if (resultOffset) {
                    if (!lastResultInstIndex)
                        generateInstruction(IST_Mov, resultOffset, ((ExprToken*)vectorBack(exprVector))->stackOffset, 0);
                    else
                        modifyExprInstResult(lastResultInstIndex, resultOffset);

                    if (getError())
                        return 0;
                }

                return ((ExprToken*)vectorBack(exprVector))->stackOffset;
            }

            topToken = &endToken;
        }

        switch (precedenceTable[tokenTypeToExprType(topToken->token->type)][tokenTypeToExprType(currentToken->type)]) {
            case Low:
            case Equal: {
                vectorPushDefaultExprToken(exprVector);
                ExprToken *exprToken = vectorBack(exprVector);
                exprToken->token = (Token*)currentToken;
                exprToken->type = Terminal;
                break;
            }
            case High:
                if (!reduce(topToken))
                    return 0;

                continue;
            case Error:
                setError(ERR_Syntax);
                return 0;
        }

        if (!endOfInput)
            tokensIt++;
    }
    return 0;
}
