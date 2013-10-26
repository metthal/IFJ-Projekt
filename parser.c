#include "scanner.h"
#include "parser.h"
#include "nierr.h"

#include <stdlib.h>
#include <stdint.h>

static Token *token = NULL;

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

void parse(const char *fileName)
{
    scannerOpenFile(fileName);
    prog();
}

void prog()
{
    token = scannerGetToken();
    if (getError())
        return;

    switch (token->type) {
        case STT_Php:
            // Rule 1
            token = scannerGetToken();
            if (getError())
                return;

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
    switch (token->type) {
        case STT_EOF:
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
            switch (token->keywordType) {
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
    // Callable only from Rule 3 with terminal function
    // doesn't need switch.
    token = scannerGetToken();
    if (getError())
        return;

    if (token->type != STT_Identifier) {
        setError(ERR_Syntax);
        return;
    }

    token = scannerGetToken();
    if (getError())
        return;

    if (token->type != STT_LeftBracket) {
        setError(ERR_Syntax);
        return;
    }

    token = scannerGetToken();
    if (getError())
        return;

    paramList();
    if (getError())
        return;

    // Right bracket loaded by paramList
    if (token->type != STT_RightBracket) {
        setError(ERR_Syntax);
        return;
    }

    token = scannerGetToken();
    if (getError())
        return;

    stmtListBracketed();
    if (getError())
        return;
}

void stmtList()
{
    switch (token->type) {
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
            switch (token->keywordType) {
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
    switch (token->type) {
        case STT_Variable:
            // Rule 8
            token = scannerGetToken();
            if (getError())
                return;

            assignment();
            if (getError())
                return;

            // Semicolon loaded by assignment
            if (token->type != STT_Semicolon) {
                setError(ERR_Syntax);
                return;
            }

            token = scannerGetToken();
            if (getError())
                return;

            break;

        case STT_Keyword:
            switch (token->keywordType) {
                case KTT_Return:
                    // Rule 9
                    token = scannerGetToken();
                    if (getError())
                        return;

                    expr();
                    if (getError())
                        return;

                    // Semicolon loaded by expr
                    if (token->type != STT_Semicolon) {
                        setError(ERR_Syntax);
                        return;
                    }

                    token = scannerGetToken();
                    if (getError())
                        return;

                    break;

                case KTT_Break:
                case KTT_Continue:
                    // Rule 10, 11
                    token = scannerGetToken();
                    if (getError())
                        return;

                    if (token->type != STT_Semicolon) {
                        setError(ERR_Syntax);
                        return;
                    }

                    token = scannerGetToken();
                    if (getError())
                        return;

                    break;

                case KTT_If:
                    // Rule 12
                    token = scannerGetToken();
                    if (getError())
                        return;

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
                    token = scannerGetToken();
                    if (getError())
                        return;

                    condition();
                    if (getError())
                        return;

                    stmtListBracketed();
                    if (getError())
                        return;

                    break;

                case KTT_For:
                    // Rule 14
                    token = scannerGetToken();
                    if (getError())
                        return;

                    if (token->type != STT_LeftBracket) {
                        setError(ERR_Syntax);
                        return;
                    }

                    token = scannerGetToken();
                    if (getError())
                        return;

                    forStmt1();
                    if (getError())
                        return;

                    // Semicolon loaded by forStmt1
                    if (token->type != STT_Semicolon) {
                        setError(ERR_Syntax);
                        return;
                    }

                    token = scannerGetToken();
                    if (getError())
                        return;

                    forStmt2();
                    if (getError())
                        return;

                    // Semicolon loaded by forStmt2
                    if (token->type != STT_Semicolon) {
                        setError(ERR_Syntax);
                        return;
                    }

                    token = scannerGetToken();
                    if (getError())
                        return;

                    forStmt1();
                    if (getError())
                        return;

                    // Right bracket loaded by forStmt2
                    if (token->type != STT_RightBracket) {
                        setError(ERR_Syntax);
                        return;
                    }

                    token = scannerGetToken();
                    if (getError())
                        return;

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
    switch (token->type) {
        case STT_EOF:
        case STT_Variable:
            // Rule 15
            break;

        case STT_Keyword:
            switch (token->keywordType) {
                case KTT_Function:
                case KTT_Return:
                case KTT_Break:
                case KTT_Continue:
                case KTT_If:
                case KTT_Else:
                case KTT_While:
                case KTT_For:
                    // Rule 15
                    break;

                case KTT_Elseif:
                    // Rule 16 (almost same as Rule 12)
                    token = scannerGetToken();
                    if (getError())
                        return;

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
    switch (token->type) {
        case STT_EOF:
        case STT_Variable:
            // Rule 17
            break;

        case STT_Keyword:
            switch (token->keywordType) {
                case KTT_Function:
                case KTT_Return:
                case KTT_Break:
                case KTT_Continue:
                case KTT_If:
                case KTT_While:
                case KTT_For:
                    // Rule 17
                    break;

                case KTT_Else:
                    // Rule 18
                    token = scannerGetToken();
                    if (getError())
                        return;

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
    switch (token->type) {
        case STT_RightBracket:
            // Rule 19
            break;

        case STT_Variable:
            // Rule 20
            token = scannerGetToken();
            if (getError())
                return;

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
    switch (token->type) {
        case STT_RightBracket:
            // Rule 19
            break;

        case STT_Comma:
            // Rule 20
            token = scannerGetToken();
            if (getError())
                return;

            if (token->type != STT_Variable) {
                setError(ERR_Syntax);
                return;
            }

            token = scannerGetToken();
            if (getError())
                return;

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
    switch (token->type) {
        case STT_RightBracket:
        case STT_Semicolon:
            break;

        case STT_Variable:
            // Rule 24 (almost same as the rule 8, missing just semicolon)
            token = scannerGetToken();
            if (getError())
                return;

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
    switch (token->type) {
        case STT_Semicolon:
            break;

        default:
            // @TODO Tokens for top down parsing doesn't have
            // to be send as it will be certainly syntax error.
            // Need predict(EXPR) to determine which.
            expr();
            if (getError())
                return;
    }
}

// @TODO implement bottom up parser for expression
void expr()
{
    while (1) {
        switch (token->type) {
            case STT_Semicolon:
            case STT_RightBracket:
            case STT_EOF:
                return;

            default:
                break;
        }

        token = scannerGetToken();
        if (getError())
            return;
    }
}

void condition()
{
    if (token->type != STT_LeftBracket) {
        setError(ERR_Syntax);
        return;
    }

    token = scannerGetToken();
    if (getError())
        return;

    expr();
    if (getError())
        return;

    // Right bracket loaded by expr
    if (token->type != STT_RightBracket) {
        setError(ERR_Syntax);
        return;
    }

    token = scannerGetToken();
    if (getError())
        return;
}

void stmtListBracketed()
{
    if (token->type != STT_LeftCurlyBracket) {
        setError(ERR_Syntax);
        return;
    }

    token = scannerGetToken();
    if (getError())
        return;

    stmtList();
    if (getError())
        return;

    // Right curly bracket loaded by stmtList
    if (token->type != STT_RightCurlyBracket) {
        setError(ERR_Syntax);
        return;
    }

    token = scannerGetToken();
    if (getError())
        return;
}

void assignment()
{
    if (token->type != STT_Equal) {
        setError(ERR_Syntax);
        return;
    }

    token = scannerGetToken();
    if (getError())
        return;

    expr();
    if (getError())
        return;
}
