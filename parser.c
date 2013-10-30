#include "parser.h"
#include "token_vector.h"
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
void elseifStmt();
void elseStmt();
void paramList();
void nparamList();
void forStmt1();
void forStmt2();
void expr();

// Forward declaration of helper functions
void condition();
void stmtListBracketed();
void assignment();

const Vector* tokens = NULL;
// Safe to iterate without range checks because last and least
// one token will be EOF, therefore range is ensured implicitly
// by grammar rules.
ConstTokenVectorIterator tokensIt = NULL;

void parse(Vector* tokenVector)
{
    tokens = tokenVector;
    tokensIt = vectorBeginToken(tokenVector);
    prog();
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

    tokensIt++;

    if (tokensIt->type != STT_LeftBracket) {
        setError(ERR_Syntax);
        return;
    }

    tokensIt++;

    paramList();
    if (getError())
        return;

    // Right bracket loaded by paramList
    if (tokensIt->type != STT_RightBracket) {
        setError(ERR_Syntax);
        return;
    }

    tokensIt++;

    stmtListBracketed();
    if (getError())
        return;
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
        case STT_Variable:
            // Rule 8
printf("%s\n", "Rule 8");
            tokensIt++;

            assignment();
            if (getError())
                return;

            // Semicolon loaded by assignment
            if (tokensIt->type != STT_Semicolon) {
                setError(ERR_Syntax);
                return;
            }

            tokensIt++;

            break;

        case STT_Keyword:
            switch (tokensIt->keywordType) {
                case KTT_Return:
                    // Rule 9
printf("%s\n", "Rule 9");
                    tokensIt++;

                    expr();
                    if (getError())
                        return;

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

                    tokensIt++;

                    break;

                case KTT_If:
                    // Rule 12
printf("%s\n", "Rule 12");
                    tokensIt++;

                    condition();
                    if (getError())
                        return;

                    stmtListBracketed();
                    if (getError())
                        return;

                    elseifStmt();
                    if (getError())
                        return;
                    // @TODO: move to Rule 15 (test if grammar still LL(1))
                    elseStmt();
                    if (getError())
                        return;

                    break;

                case KTT_While:
                    // Rule 13 (almost same as Rule 12, missing just elseif call)
printf("%s\n", "Rule 13");
                    tokensIt++;

                    condition();
                    if (getError())
                        return;

                    stmtListBracketed();
                    if (getError())
                        return;

                    break;

                case KTT_For:
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

                    forStmt2();
                    if (getError())
                        return;

                    // Semicolon loaded by forStmt2
                    if (tokensIt->type != STT_Semicolon) {
                        setError(ERR_Syntax);
                        return;
                    }

                    tokensIt++;

                    forStmt1();
                    if (getError())
                        return;

                    // Right bracket loaded by forStmt2
                    if (tokensIt->type != STT_RightBracket) {
                        setError(ERR_Syntax);
                        return;
                    }

                    tokensIt++;

                    stmtListBracketed();
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

void elseifStmt()
{
    switch (tokensIt->type) {
        case STT_EOF:
        case STT_Variable:
        case STT_RightCurlyBracket:
            // Rule 15
printf("%s\n", "Rule 15");
            break;

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
                    break;

                case KTT_Elseif:
                    // Rule 16 (almost same as Rule 12)
printf("%s\n", "Rule 16");
                    tokensIt++;

                    condition();
                    if (getError())
                        return;

                    stmtListBracketed();
                    if (getError())
                        return;

                    elseifStmt();
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

void elseStmt()
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
                    tokensIt++;

                    stmtListBracketed();
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
            tokensIt++;

            nparamList();
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

            tokensIt++;

            nparamList();
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
            tokensIt++;

            assignment();
            if (getError())
                return;

            break;

        default:
            setError(ERR_Syntax);
    }
}

void forStmt2()
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
            expr();
            if (getError())
                return;
    }
}

// @TODO implement bottom up parser for expression
void expr()
{
    int leftBrackets = 0;
    while (1) {
        switch (tokensIt->type) {
            case STT_Semicolon:
            case STT_EOF:
                return;

            case STT_LeftBracket:
                leftBrackets++;
                break;

            case STT_RightBracket:
                if(leftBrackets == 0)
                    return;
                leftBrackets--;
                break;

            default:
                break;
        }

        tokensIt++;
    }
}

void condition()
{
    if (tokensIt->type != STT_LeftBracket) {
        setError(ERR_Syntax);
        return;
    }

    tokensIt++;

    expr();
    if (getError())
        return;

    // Right bracket loaded by expr
    if (tokensIt->type != STT_RightBracket) {
        setError(ERR_Syntax);
        return;
    }

    tokensIt++;
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

void assignment()
{
    if (tokensIt->type != STT_Assignment) {
        setError(ERR_Syntax);
        return;
    }

    tokensIt++;

    expr();
    if (getError())
        return;
}
