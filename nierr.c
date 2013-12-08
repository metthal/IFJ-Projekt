#include "nierr.h"
/** Global interpret error variable. */
NiError niErr;

#ifdef DEBUG
#include "token_vector.h"
#include <unistd.h>

#define STD_ERR_INT     2
#define MAX_TOKEN_PEEK  4
#define NORMAL_COLOR    "\033[0m"
#define ERR_COLOR       "\033[1m\033[31m"
#define INFO_COLOR      "\033[1m\033[33m"

char errorPrinted = 1;

extern ConstTokenVectorIterator tokensIt;

void printToken(const Token *token) {
    switch (token->type) {
        case STT_Plus:
            fprintf(stderr, "+"); break;
        case STT_Minus:
            fprintf(stderr, "-"); break;
        case STT_Multiply:
            fprintf(stderr, "*"); break;
        case STT_Divide:
            fprintf(stderr, "/"); break;
        case STT_Dot:
            fprintf(stderr, "."); break;
        case STT_Less:
            fprintf(stderr, "<"); break;
        case STT_Greater:
            fprintf(stderr, ">"); break;
        case STT_LessEqual:
            fprintf(stderr, "<="); break;
        case STT_GreaterEqual:
            fprintf(stderr, ">="); break;
        case STT_Equal:
            fprintf(stderr, "==="); break;
        case STT_NotEqual:
            fprintf(stderr, "!=="); break;
        case STT_LeftBracket:
            fprintf(stderr, "("); break;
        case STT_RightBracket:
            fprintf(stderr, ")"); break;
        case STT_Identifier:
            fprintf(stderr, "%s", token->str.data); break;
        case STT_Comma:
            fprintf(stderr, ","); break;
        case STT_EOF:
            fprintf(stderr, "EOF"); break;
        case STT_Variable:
            fprintf(stderr, "$%s", token->str.data); break;
        case STT_Number:
            fprintf(stderr, "%d", token->n); break;
        case STT_Double:
            fprintf(stderr, "%g", token->d); break;
        case STT_Bool:
            fprintf(stderr, "%s", token->n == 0 ? "false" : "true"); break;
        case STT_Null:
            fprintf(stderr, "null"); break;
        case STT_String:
            fprintf(stderr, "\"%s\"", token->str.data); break;
        case STT_Semicolon:
            fprintf(stderr, ";"); break;
        case STT_LeftCurlyBracket:
            fprintf(stderr, "{"); break;
        case STT_RightCurlyBracket:
            fprintf(stderr, "}"); break;
        case STT_Empty:
            break;
        case STT_Keyword:
            switch (token->keywordType) {
                case KTT_If:
                    fprintf(stderr, "if"); break;
                case KTT_Else:
                    fprintf(stderr, "else"); break;
                case KTT_Elseif:
                    fprintf(stderr, "elseif"); break;
                case KTT_Continue:
                    fprintf(stderr, "continue"); break;
                case KTT_Break:
                    fprintf(stderr, "break"); break;
                case KTT_While:
                    fprintf(stderr, "while"); break;
                case KTT_For:
                    fprintf(stderr, "for"); break;
                case KTT_Return:
                    fprintf(stderr, "return"); break;
                case KTT_Function:
                    fprintf(stderr, "function"); break;
                default:
                    break;
            }
            break;
        case STT_Assignment:
            fprintf(stderr, "="); break;
        case STT_Php:
            fprintf(stderr, "<?php"); break;
        case STT_And:
            fprintf(stderr, "&&"); break;
        case STT_Or:
            fprintf(stderr, "||"); break;
        case STT_Not:
            fprintf(stderr, "!"); break;
        default:
            break;
    }
}

void printErrorF(const char *str)
{
    if (isatty(STD_ERR_INT)) {
        fprintf(stderr, ERR_COLOR "Error at %s:%d:%s: %s" INFO_COLOR,
                niErr.file, niErr.line, niErr.fun, str);
    }
    else {
        fprintf(stderr, "Error at %s:%d:%s: %s",
                niErr.file, niErr.line, niErr.fun, str);
    }

    if ((niErr.type == ERR_Syntax || niErr.type == ERR_UndefVariable) && (tokensIt != NULL)) {
        fprintf(stderr, "Tokens around error:\n");
        ConstTokenVectorIterator beginIt = tokensIt;
        ConstTokenVectorIterator endIt = tokensIt;

        for (int i = 0; beginIt->type != STT_Php && i < MAX_TOKEN_PEEK; i++)
            beginIt--;
        for (int i = 0; endIt->type != STT_EOF && i < MAX_TOKEN_PEEK; i++)
            endIt++;
        endIt++; // To point behind last token

        for (;beginIt != endIt; beginIt++) {
            printToken(beginIt);
            fprintf(stderr, " ");
        }
        fprintf(stderr, "\n");
    }

    if (isatty(STD_ERR_INT)) {
        fprintf(stderr, NORMAL_COLOR);
    }
}

void printError()
{
    if (niErr.type == ERR_None || errorPrinted)
        return;


    switch (niErr.type) {
        case ERR_Unknown:
            printErrorF("Unknown error.\n");
            break;
        case ERR_LexFile:
            printErrorF("Scanner couldn't process file.\n");
            break;
        case ERR_Allocation:
            printErrorF("Allocation failed.\n");
            break;
        case ERR_Range:
            printErrorF("Index was outside of allowed range.\n");
            break;
        case ERR_Internal:
            printErrorF("Internal error.\n");
            break;
        case ERR_Convert:
            printErrorF("Conversion failed.\n");
            break;
        case ERR_Syntax:
            printErrorF("Syntax error.\n");
            break;
        case ERR_UndefFunction:
            printErrorF("Undefined function used.\n");
            break;
        case ERR_RedefFunction:
            printErrorF("Redefinition of function.\n");
            break;
        case ERR_UndefVariable:
            printErrorF("Undefined variable used.\n");
            break;
        case ERR_RedefParameter:
            printErrorF("Redefinition of function parameter.\n");
            break;
        case ERR_BadParamCount:
            printErrorF("Function called with wrong number of parameters.\n");
            break;
        case ERR_DefArgOrder:
            printErrorF("Non-default parameter appeared after default one.\n");
            break;
        case ERR_BadDefArg:
            printErrorF("Invalid default value of parameter.\n");
            break;
        case ERR_ISTGenerator:
            printErrorF("Instruction generator error.\n");
            break;
        case ERR_CycleControl:
            printErrorF("Break or continue used outside cycle.\n");
            break;
        case ERR_Substr:
            printErrorF("Substring parameters are wrong.\n");
            break;
        case ERR_OperandTypes:
            printErrorF("Invalid operand type.\n");
            break;
        case ERR_DivideByZero:
            printErrorF("Division by zero.\n");
            break;
        default:
            printErrorF("Undocumented error.\n");
            break;
    }

    errorPrinted = 1;
}

#endif
