#ifndef SCANNER_H
#define SCANNER_H

#include "string.h"

typedef enum
{
    KTT_If,
    KTT_Else,
    KTT_Elseif,
    KTT_Continue,
    KTT_Break,
    KTT_While,
    KTT_For,
    KTT_True,
    KTT_False,
    KTT_Return,
    KTT_Function,
    KTT_Null,
} KeywordTokenType;

typedef enum
{
    STT_Semicolon,
    STT_LeftCurlyBracket,
    STT_Asterisk,
    STT_RightCurlyBracket,
    STT_Empty,
    STT_Plus,
    STT_Minus,
    STT_RightBracket,
    STT_LeftBracket,
    STT_Identifier,
    STT_Variable,
    STT_Number,
    STT_Keyword,
    STT_Comma,
    STT_Assignment,
    STT_Greater,
    STT_GreaterEqual,
    STT_Less,
    STT_LessEqual,
    STT_Equal,
    STT_LexError,
    STT_EOF,
    STT_Double,
    STT_Space,
    STT_Backslash,
    STT_Divide,
    STT_NotEqual,
    STT_String,
    STT_Php,
    STT_Dot
} ScannerTokenType;

typedef enum
{
    STS_FINISHED,
    STS_NOT_FINISHED,
} ScannerTokenState;

typedef enum
{
    SS_Identifier,
    SS_Dollar,
    SS_Number,
    SS_Greater,
    SS_Less,
    SS_Assignment,
    SS_Equal,
    SS_Empty,
    SS_Variable,
    SS_Double,
    SS_Divide,
    SS_BlockComment,
    SS_BlockCommentFinish,
    SS_Comment,
    SS_Exclamation,
    SS_NotEqual,
    SS_String,
} ScannerState;

typedef struct
{
    ScannerTokenType type;
    union
    {
        double d;
        int n;
        KeywordTokenType keywordType;
        String str;
    };
} Token;

void checkKeyword(String *str, Token *token);
Token* scannerGetToken();
Token* newToken();
void initToken(Token *pt);
void deleteToken(Token *pt);
void freeToken(Token **ppt);

void scannerReset();
FILE* scannerOpenFile(const char *fileName);

#endif

