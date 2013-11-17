#ifndef TOKEN_H
#define TOKEN_H

#include "string.h"

typedef enum
{
    KTT_None,
    KTT_If,
    KTT_Else,
    KTT_Elseif,
    KTT_Continue,
    KTT_Break,
    KTT_While,
    KTT_For,
    KTT_Return,
    KTT_Function,
    // Helper keywords that gets converted to value
    KTT_True,
    KTT_False,
    KTT_Null
} KeywordTokenType;

typedef enum
{
    STT_Plus = 0,
    STT_Minus,
    STT_Multiply,
    STT_Divide,
    STT_Dot,
    STT_Less,
    STT_Greater,
    STT_LessEqual,
    STT_GreaterEqual,
    STT_Equal,
    STT_NotEqual,
    STT_LeftBracket,
    STT_RightBracket,
    STT_Identifier,
    STT_Comma,
    STT_EOF,
    STT_Variable,
    STT_Number,
    STT_Double,
    STT_Bool,
    STT_Null,
    STT_String,
    STT_Semicolon,
    STT_LeftCurlyBracket,
    STT_RightCurlyBracket,
    STT_Empty,
    STT_Keyword,
    STT_Assignment,
    STT_Php
} ScannerTokenType;

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

/**
 * Constructs new token on heap and initializes it
 * to default state.
 * @return Constructed token.
 */
Token* newToken();

/**
 * Initializes already constructed token to default
 * state. Token must be in uninitialized state.
 * @param pt Token to initialize.
 */
void initToken(Token *pt);

/**
 * Deletes content of token, freeing all memory
 * allocated by it's members and token itself.
 * Leaves token in uninitialized state.
 * @param pt Token to delete.
 */
void deleteToken(Token *pt);

/**
 * Destroys token on heap, freeing all memory
 * allocated by it's members and token itself.
 * The pointer to token is set to NULL afterwards.
 * @param ppt Pointer to token to free.
 */
void freeToken(Token **ppt);

/**
 * Deep-copies the content of source to destination
 * token. Destination token must be in just-constructed
 * or uninitialized state.
 * @param src Source token that will be copied.
 * @param dest Destination token that will be copy.
 */
void copyToken(Token *src, Token *dest);

#endif
