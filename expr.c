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
    endToken.subtype = Const;
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

static inline void modifyExprInstResult(uint32_t instIndex, int64_t resultOffset)
{
    Instruction *inst = vectorAt(instructions, instIndex);

    switch (inst->code) {
        case IST_Reserve:
            inst->code = IST_PushRef;
            inst->a = resultOffset - (currentStackPos - 1);
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
        case IST_And:
        case IST_Not:
            // currentStackPos points to the first free stack position, decrement it so we can properly set maxStackPos for top-down parser
            if (inst->res > inst->a && inst->res > inst->b)
                currentStackPos--;
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
uint8_t reduceMultiparamFunc(int64_t stackPos, uint32_t *paramCount, uint32_t *totalParamCount)
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
        generateInstruction(first->subtype == Const ? IST_PushC : IST_Push, ISM_NoConst, 0, first->offset, 0);
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

            topTerm->type = NonTerminal;
            topTerm->subtype = Const;
            topTerm->offset = vectorSize(constantsTable) - 1;
            break;
        case STT_Variable: {
            Symbol *symbol = symbolTableFind(currentContext->localTable, &(topTerm->token->str));
            if (symbol == NULL) {
                setError(ERR_UndefVariable);
                return 0;
            }

            topTerm->type = NonTerminal;
            topTerm->subtype = NonConst;
            topTerm->offset = symbol->data->var.relativeIndex;
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

            // for non-const generate IST_Not instructions
            // for const perform NOT right here and store it in the const table
            if (operand->subtype == NonConst) {
                if (operand->offset >= currentContext->exprStart)
                    notOperator->offset = operand->offset;
                else
                    notOperator->offset = currentStackPos++;

                generateInstruction(IST_Not, ISM_NoConst, notOperator->offset, operand->offset, notCount & 1); // same as notCount % 2
            }
            else {
                Value *constVal = vectorAt(constantsTable, operand->offset);
                uint8_t tmp = valueToBool(constVal) ^ (notCount & 1); // this mechanism explained in interpreter.c case IST_Not
                deleteValue(constVal);
                constVal->type = VT_Bool;
                constVal->data.b = tmp;
                notOperator->offset = operand->offset;
                notOperator->subtype = Const;
            }

            if (getError())
                return 0;

            notOperator->type = NonTerminal;
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

            uint8_t modMask = ISM_NoConst;
            if (operand1->subtype == Const)
                modMask |= ISM_FirstConst;

            if (operand2->subtype == Const)
                modMask |= ISM_SecondConst;

            if (!(modMask & ISM_FirstConst) && operand1->offset >= currentContext->exprStart) {
                generateInstruction(tokenTypeToInstruction(operator->token->type), modMask, operand1->offset, operand1->offset, operand2->offset);
            }
            else if (!(modMask & ISM_SecondConst) && operand2->offset >= currentContext->exprStart) {
                generateInstruction(tokenTypeToInstruction(operator->token->type), modMask, operand2->offset, operand1->offset, operand2->offset);
                operand1->offset = operand2->offset;
            }
            else {
                generateInstruction(tokenTypeToInstruction(operator->token->type), modMask, currentStackPos, operand1->offset, operand2->offset);
                operand1->offset = currentStackPos++;
            }

            if (getError())
                return 0;

            operand1->subtype = NonConst;
            lastResultInstIndex = vectorSize(instructions) - 1;
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
                        generateInstruction(IST_Reserve, ISM_NoConst, 0, 1, 0);
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
                        nextTerm->subtype = NonConst;
                        nextTerm->offset = retValStackPos;
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
                        generateInstruction(IST_Reserve, ISM_NoConst, 0, 1, 0);
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
                        nextTerm->subtype = NonConst;
                        nextTerm->offset = retValStackPos;
                    }
                    else if (nextTerm->type == Terminal && nextTerm->token->type == STT_LeftBracket) { // it's (E) and we don't know if it is single param func or just (E)
                        if (stackSize >= 4) {
                            ExprTokenVectorIterator leftBracketBackup = nextTerm; // we need to backup current token for (E) case
                            nextTerm--;

                            if (nextTerm->type == Terminal && nextTerm->token->type == STT_Identifier) { // we have id(E) and it's function
                                // return value
                                uint32_t retValStackPos = currentStackPos++;
                                generateInstruction(IST_Reserve, ISM_NoConst, 0, 1, 0);
                                if (getError())
                                    return 0;

                                lastResultInstIndex = vectorSize(instructions) - 1;
                                currentFuncSymbol = fillInstFuncInfo(nextTerm->token, &currentFuncBuiltinCode, &currentFuncParamLimit);
                                if (getError())
                                    return 0;

                                // if there is no parameter limit or it is higher than 0
                                if (currentFuncParamLimit == -1 || currentFuncParamLimit > 0) {
                                    generateInstruction(exprBackup->subtype == Const ? IST_PushC : IST_Push, ISM_NoConst, 0, exprBackup->offset, 0);
                                    if (getError())
                                        return 0;
                                }

                                lastFuncParamCount = generateCall(currentFuncSymbol, currentFuncBuiltinCode, currentFuncParamLimit == 0 ? 0 : 1);
                                if (getError())
                                    return 0;

                                nextTerm->type = NonTerminal;
                                nextTerm->subtype = NonConst;
                                nextTerm->offset = retValStackPos;
                                vectorPopNExprToken(exprVector, 3);
                            }
                            else { // it's just (E)
                                leftBracketBackup->type = NonTerminal;
                                leftBracketBackup->subtype = exprBackup->subtype;
                                leftBracketBackup->offset = exprBackup->offset;
                                vectorPopNExprToken(exprVector, 2);
                            }
                        }
                        else { // it can only be (E) if we have just 3 items on the stack
                            nextTerm->type = NonTerminal;
                            nextTerm->subtype = exprBackup->subtype;
                            nextTerm->offset = exprBackup->offset;
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
int64_t expr(int64_t resultOffset, uint32_t *maxStackPosUsed)
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
                ExprToken *expr = vectorBack(exprVector);
                if (resultOffset) {
                    if (!lastResultInstIndex)
                        generateInstruction(expr->subtype == Const ? IST_MovC : IST_Mov, ISM_NoConst, resultOffset, expr->offset, 0);
                    else
                        modifyExprInstResult(lastResultInstIndex, resultOffset);

                    if (getError())
                        return 0;
                }

                if (maxStackPosUsed)
                    *maxStackPosUsed = currentStackPos;
                return expr->offset;
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
                exprToken->subtype = Const;
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
