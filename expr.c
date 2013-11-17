#include "expr.h"
#include "nierr.h"
#include "token_vector.h"

// definitions from parser which need to be present also in expression parser
extern ConstTokenVectorIterator tokensIt;

static Vector *exprVector = NULL;
static ExprToken endToken;

typedef enum
{
    Low,
    High,
    Equal,
    Error
} TokenPrecedence;

static uint8_t precedenceTable[STT_Semicolon][STT_Semicolon] =
{
//      +       -       *       /       .       <       >       <=      >=      ===     !==     (       )       func    ,       $       var
    {   High,   High,   Low,    Low,    Low,    High,   High,   High,   High,   High,   High,   Low,    High,   Low,    High,   High,   Low   },  // +
    {   High,   High,   Low,    Low,    Low,    High,   High,   High,   High,   High,   High,   Low,    High,   Low,    High,   High,   Low   },  // -
    {   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   Low,    High,   Low,    High,   High,   Low   },  // *
    {   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   Low,    High,   Low,    High,   High,   Low   },  // /
    {   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   Low,    High,   Low,    High,   High,   Low   },  // .
    {   Low,    Low,    Low,    Low,    Low,    High,   High,   High,   High,   High,   High,   Low,    High,   Low,    High,   High,   Low   },  // <
    {   Low,    Low,    Low,    Low,    Low,    High,   High,   High,   High,   High,   High,   Low,    High,   Low,    High,   High,   Low   },  // >
    {   Low,    Low,    Low,    Low,    Low,    High,   High,   High,   High,   High,   High,   Low,    High,   Low,    High,   High,   Low   },  // <=
    {   Low,    Low,    Low,    Low,    Low,    High,   High,   High,   High,   High,   High,   Low,    High,   Low,    High,   High,   Low   },  // >=
    {   Low,    Low,    Low,    Low,    Low,    High,   High,   High,   High,   High,   High,   Low,    High,   Low,    High,   High,   Low   },  // ===
    {   Low,    Low,    Low,    Low,    Low,    High,   High,   High,   High,   High,   High,   Low,    High,   Low,    High,   High,   Low   },  // !==
    {   Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Equal,  Low,    Equal,  Error,  Low   },  // (
    {   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   Error,  High,   Error,  High,   High,   Error },  // )
    {   Error,  Error,  Error,  Error,  Error,  Error,  Error,  Error,  Error,  Error,  Error,  Equal,  Error,  Error,  Error,  Error,  Error },  // func
    {   Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Equal,  Low,    Equal,  High,   Low   },  // ,
    {   Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Low,    Error,  Low,    Error,  Error,  Low   },  // $
    {   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   High,   Error,  High,   Error,  High,   High,   Error }   // var
};

void initExpr()
{
    exprVector = newExprTokenVector();
    endToken.token = newToken();
    endToken.token->type = STT_EOF;
    endToken.type = Terminal;
}

void deinitExpr()
{
    free(endToken.token);
    vectorClearExprToken(exprVector);
    freeExprTokenVector(&exprVector);
}

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

static inline uint8_t tokenTypeToExprType(uint8_t tokenType)
{
    if (tokenType >= STT_Semicolon)
        return STT_EOF;

    if (tokenType >= STT_Variable && tokenType <= STT_String)
        return STT_Variable;

    return tokenType;
}

uint8_t reduce(ExprToken *topTerm)
{
    switch (topTerm->token->type) {
        case STT_Variable:
            // todo: generate instruction
puts("Rule: E -> var");
            topTerm->type = NonTerminal;
            break;
        case STT_Number:
puts("Rule: E -> int");
            topTerm->type = NonTerminal;
            break;
        case STT_Double:
puts("Rule: E -> double");
            topTerm->type = NonTerminal;
            break;
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
if (topTerm->token->type == STT_Plus)
    puts("Rule: E -> E + E");
else if (topTerm->token->type == STT_Dot)
    puts("Rule: E -> E . E");
else if (topTerm->token->type == STT_Divide)
    puts("Rule: E -> E / E");
else if (topTerm->token->type == STT_Multiply)
    puts("Rule: E -> E * E");
else if (topTerm->token->type == STT_Minus)
    puts("Rule: E -> E - E");
else if (topTerm->token->type == STT_Equal)
    puts("Rule: E -> E === E");
else if (topTerm->token->type == STT_NotEqual)
    puts("Rule: E -> E !== E");
else if (topTerm->token->type == STT_Less)
    puts("Rule: E -> E < E");
else if (topTerm->token->type == STT_Greater)
    puts("Rule: E -> E > E");
else if (topTerm->token->type == STT_LessEqual)
    puts("Rule: E -> E <= E");
else if (topTerm->token->type == STT_GreaterEqual)
    puts("Rule: E -> E >= E");

            uint32_t stackSize = vectorSize(exprVector);
            if (stackSize < 3) {
                setError(ERR_Syntax);
                return 0;
            }

            ExprTokenVectorIterator operand1 = vectorAt(exprVector, stackSize - 1);
            if (operand1->type != NonTerminal) {
                setError(ERR_Syntax);
                return 0;
            }

            ExprTokenVectorIterator operator = vectorAt(exprVector, stackSize - 2);
            if (operator != topTerm) {
                setError(ERR_Syntax);
                return 0;
            }

            ExprTokenVectorIterator operand2 = vectorAt(exprVector, stackSize - 3);
            if (operand2->type != NonTerminal) {
                setError(ERR_Syntax);
                return 0;
            }

            vectorPopExprToken(exprVector);
            vectorPopExprToken(exprVector);
            break;
        }
        case STT_RightBracket: {
puts("Rule: E -> (E)");

            uint32_t stackSize = vectorSize(exprVector);
            if (stackSize < 3) {
                setError(ERR_Syntax);
                return 0;
            }

            ExprTokenVectorIterator rightBracket = vectorAt(exprVector, stackSize - 1);
            if (rightBracket != topTerm) {
                setError(ERR_Syntax);
                return 0;
            }

            ExprTokenVectorIterator expr = vectorAt(exprVector, stackSize - 2);
            if (expr->type != NonTerminal) {
                setError(ERR_Syntax);
                return 0;
            }

            ExprTokenVectorIterator leftBracket = vectorAt(exprVector, stackSize - 3);
            if (leftBracket->type != Terminal && leftBracket->token->type != STT_LeftBracket) {
                setError(ERR_Syntax);
                return 0;
            }

            vectorPopExprToken(exprVector);
            vectorPopExprToken(exprVector);
            leftBracket->type = NonTerminal;
            break;
        }
        default:
            return 0;
    }

    return 1;
}

void initExprToken(ExprToken *token)
{
    memset(token, 0, sizeof(ExprToken));
}

void deleteExprToken(ExprToken *token)
{
    (void)token;
}

void copyExprToken(ExprToken *src, ExprToken *dest)
{
    dest->type = src->type;
    dest->token = src->token;
}

int32_t expr()
{
    // insert bottom of the stack (special kind of token) to the stack
    uint8_t endOfInput = 0;
    const Token *currentToken = NULL;
    vectorClearExprToken(exprVector);

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
            if (endOfInput && vectorSize(exprVector) == 1 && ((ExprToken*)vectorBack(exprVector))->type == NonTerminal)
                return 1;

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
                // @todo Cleanup
                return -1;
        }

        if (!endOfInput)
            tokensIt++;
    }
}
