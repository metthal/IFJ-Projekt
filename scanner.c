#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include "scanner.h"
#include "string.h"
#include "convert.h"
#include "nierr.h"

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
    SS_Php_0,
    SS_Php_1,
    SS_Php_2,
    SS_Php_3,
} ScannerState;

static int32_t charStreamSwitch = 1;
static int32_t lastChar = 0;
static FILE *source = 0;

Token* newToken() // FUNCTION CREATES NEWTOKEN STRUCTURE
{
    Token* tmp = malloc(sizeof(Token));
    tmp->type = STT_Empty;
    return tmp;
}

void initToken(Token *pt)
{
    pt->type = STT_Empty;
}

void deleteToken(Token *pt)
{
    if (pt != NULL) {
        if ((pt->type == STT_Variable)
            || (pt->type == STT_Identifier)
            || (pt->type == STT_String)) {
            deleteString(&pt->str);
        }
    }
}

void freeToken(Token **ppt)
{
    if (ppt != NULL) {
        if (*ppt != NULL) {
            if (((*ppt)->type == STT_Variable)
                || ((*ppt)->type == STT_Identifier)
                || ((*ppt)->type == STT_String)) {
                deleteString(&(*ppt)->str);
            }
        }
        free(*ppt);
        *ppt = NULL;
    }
}

KeywordTokenType strToKeyword(String *str)
{
    switch (str->data[0]) {
        case 'b': {
            if (stringCompareS(str, "break", 5) == 0) {
                return KTT_Break;
            }
            break;
        }
        case 'c': {
            if (stringCompareS(str, "continue", 8) == 0) {
                return KTT_Continue;
            }
            break;
        }
        case 'e': {
            if (stringCompareS(str, "else", 4) == 0) {
                return KTT_Else;
            }
            else if (stringCompareS(str, "elseif", 6) == 0){
                return KTT_Elseif;
            }
            break;
        }
        case 'f': {
            if (stringCompareS(str, "false", 5) == 0) {
                return KTT_False;
            }
            else if (stringCompareS(str, "for", 3) == 0){
                return KTT_For;
            }
            else if (stringCompareS(str, "function", 8) == 0){
                return KTT_Function;
            }
            break;
        }
        case 'i': {
            if (stringCompareS(str, "if", 2) == 0) {
                return KTT_If;
            }
            break;
        }
        case 'n': {
            if (stringCompareS(str, "null", 4) == 0) {
                return KTT_Null;
            }
            break;
        }
        case 'r': {
            if (stringCompareS(str, "return", 6) == 0) {
                return KTT_Return;
            }
            break;
        }
        case 't': {
            if (stringCompareS(str, "true", 4) == 0) {
                return KTT_True;
            }
            break;
        }
        case 'w': {
            if (stringCompareS(str, "while", 5) == 0) {
                return KTT_While;
            }
            break;
        }
    }
    return KTT_None;
}

void scannerReset()
{
    charStreamSwitch = 1;
    lastChar = 0;
    if (source != NULL) {
        fclose(source);
        source = NULL;
    }
}

FILE* scannerOpenFile(const char *fileName) // OPEN FILE AND RETURN POINTER ON IT
{
    if (source != NULL) {
        fclose(source);
        source = NULL;
    }

    source = fopen(fileName, "r");
    return source;
}

Token* scannerGetToken() // FUNCTION, WHICH RETURNS POINTER ON TOKEN STRUCTURE
{
    Token *token = newToken();

    ScannerState state = SS_Empty;
    int32_t symbol;
    int32_t firsTimeSwitch = 1;

    String *tokenStr = &(token->str);

    while (1)
    {
        if (charStreamSwitch) {
            symbol = getc(source);
        }
        else {
            symbol = lastChar;
            charStreamSwitch = 1;
        }

        switch (state) {
            case SS_Empty: {
                if ((symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z')) {
                    state = SS_Identifier;
                    initString(tokenStr);
                    stringPush(tokenStr,symbol);
                }
                else if (symbol >= '0' && symbol <= '9') {
                    state = SS_Number;
                    initString(tokenStr);
                    stringPush(tokenStr,symbol);
                }
                else if (isspace(symbol)) {
                    ;
                }
                else if (symbol == EOF) {
                    token->type = STT_EOF;
                    return token;
                }
                else {
                    switch (symbol) {
                        case '"':
                            state = SS_String;
                            initString(tokenStr);
                            break;
                        case '/':
                            state = SS_Divide;
                            break;
                        case '!':
                            state = SS_Exclamation;
                            break;
                        case ';':
                            token->type = STT_Semicolon;
                            return token;
                        case '{':
                            token->type = STT_LeftCurlyBracket;
                            return token;
                        case '}':
                            token->type = STT_RightCurlyBracket;
                            return token;
                        case '*':
                            token->type = STT_Asterisk;
                            return token;
                        case '+':
                            token->type = STT_Plus;
                            return token;
                        case '-':
                            token->type = STT_Minus;
                            return token;
                        case '(':
                            token->type = STT_LeftBracket;
                            return token;
                        case ')':
                            token->type = STT_RightBracket;
                            return token;
                        case ',':
                            token->type = STT_Comma;
                            return token;
                        case '.':
                            token->type = STT_Dot;
                            return token;
                        case '>':
                            state = SS_Greater;
                            break;
                        case '<':
                            state = SS_Less;
                            break;
                        case '=':
                            state = SS_Assignment;
                            break;
                        case '$':
                            state = SS_Dollar;
                            break;
                        default:
                            freeToken(&token);
                            setError(ERR_LexFile);
                            return NULL;
                    }
                }
                break;
            }
            case SS_Greater: {
                if (symbol == '=') {
                    token->type = STT_GreaterEqual;
                    return token;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    token->type = STT_Greater;
                    return token;
                }
            }
            case SS_Less: {
                if (symbol == '=') {
                    token->type = STT_LessEqual;
                    return token;
                }
                else if (symbol == '?') {
                    state = SS_Php_0;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    token->type = STT_Less;
                    return token;
                }
                break;
            }
            case SS_Assignment: {
                if (symbol == '=') {
                    state = SS_Equal;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    token->type = STT_Assignment;
                    return token;
                }
                break;
            }
            case SS_Dollar: {
                if ((symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z') || (symbol == '_')) {
                    state = SS_Variable;
                    initString(tokenStr);
                    stringPush(tokenStr, symbol);
                    break;
                }
                else {
                    freeToken(&token);
                    setError(ERR_LexFile);
                    return NULL;
                }
                break;
            }
            case SS_Variable: {
                if ((symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z') || (symbol == '_') ||
                     (symbol >= '0' && symbol <= '9')) {
                    stringPush(tokenStr, symbol);
                    state = SS_Variable;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    token->type = STT_Variable;
                    return token;
                }
                break;
            }
            case SS_Number: {
                if (symbol >= '0' && symbol <= '9') {
                    state = SS_Number;
                    stringPush(tokenStr, symbol);
                }
                else if (symbol == '.') {
                    state = SS_Double;
                    stringPush(tokenStr, symbol);
                }
                // TODO ELSE IF EXPONENT
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    int n = stringToInt(tokenStr);
                    deleteString(tokenStr);
                    token->n = n;
                    token->type = STT_Number;
                    return token;
                }
                break;
            }
            case SS_Identifier: {
                if ((symbol == '_') || (symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z') || (symbol >= '0' && symbol <= '9')) {
                    stringPush(tokenStr, symbol);
                    state = SS_Identifier;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    KeywordTokenType keywordType = strToKeyword(tokenStr); // FUNCTION CHECKS IF IDENTIFIER, WHICH HAS BEEN FOUND AINT A RESERVED ( KEYWORD ) WORD
                    if (keywordType == KTT_None) {
                        token->type = STT_Identifier;
                        return token;
                    } else {
                        deleteString(tokenStr);
                        token->type = STT_Keyword;
                        token->keywordType = keywordType;
                        return token;
                    }
                }
                break;
            }
            case SS_Double: {
                if (symbol >= '0' && symbol <= '9') {
                    state = SS_Double;
                    stringPush(tokenStr, symbol);
                    firsTimeSwitch = 0;
                }
                else if ((symbol == EOF) && (firsTimeSwitch)) {
                    deleteString(tokenStr);
                    freeToken(&token);
                    setError(ERR_LexFile);
                    return NULL;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    double d = stringToDouble(tokenStr);
                    deleteString(tokenStr);
                    token->d = d;
                    token->type = STT_Double;
                    return token;
                }
                break;
            }
            case SS_Divide: {
                if (symbol == '*') {
                    state = SS_BlockComment;
                }
                else if (symbol == '/') {
                    state = SS_Comment;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    token->type = STT_Divide;
                    return token;
                }
                break;
            }
            case SS_BlockComment: {
                if (symbol == '*') {
                    state = SS_BlockCommentFinish;
                }
                else if (symbol == EOF) {
                    // @TODO , IS IT MISTAKE OR NOT ?
                    token->type = STT_EOF;
                    return token;
                }
                else {
                    state = SS_BlockComment;
                }
                break;
            }
            case SS_BlockCommentFinish: {
                if (symbol == '/') {
                    state = SS_Empty;
                }
                else if (symbol == EOF) {
                    // @TODO , IS IT MISTAKE OR NOT ?
                    token->type = STT_EOF;
                    return token;
                }
                else {
                    state = SS_BlockComment;
                    lastChar = symbol;
                    charStreamSwitch = 0;
                }
                break;
            }
            case SS_Comment: {
                if (symbol == '\n') {
                    state = SS_Empty;
                }
                else if (symbol == EOF) {
                    token->type = STT_EOF;
                    return token;
                }
                else {
                    state = SS_Comment;
                }
                break;
            }
            case SS_Equal: {
                if (symbol == '=') {
                    token->type = STT_Equal;
                    return token;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    freeToken(&token);
                    setError(ERR_LexFile);
                    return NULL;
                }
            }
            case SS_Exclamation: {
                if (symbol == '=') {
                    state = SS_NotEqual;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    freeToken(&token);
                    setError(ERR_LexFile);
                    return NULL;
                }
                break;
            }
            case SS_NotEqual: {
                if (symbol == '=') {
                    token->type = STT_NotEqual;
                    return token;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    freeToken(&token);
                    setError(ERR_LexFile);
                    return NULL;
                }
            }
            case SS_String: {
                if (symbol == '"') {
                    token->type = STT_String;
                    return token;
                }
                else if (symbol == EOF) {
                    freeToken(&token);
                    setError(ERR_LexFile);
                    return NULL;
                }
                else {
                    state = SS_String;
                    stringPush(tokenStr, symbol);
                    // TODO = AUTOMAT
                }
                break;
            }
            case SS_Php_0: {
                if (symbol == 'p') {
                    state = SS_Php_1;
                }	
                else {
                    freeToken(&token);
                    setError(ERR_LexFile);
                    return NULL;	
                }
                break;
            }
            case SS_Php_1: {
                if (symbol == 'h') {
                    state = SS_Php_2;
                }	
                else {
                    freeToken(&token);
                    setError(ERR_LexFile);
                    return NULL;	
                }
                break;
            }
            case SS_Php_2: {
                if (symbol == 'p') {
                    state = SS_Php_3;
                }	
                else {
                    freeToken(&token);
                    setError(ERR_LexFile);
                    return NULL;	
                }
                break;
            }
            case SS_Php_3: {
                if (isspace(symbol)) {
                    token->type = STT_Php;
                    return token;
                }	
                else {
                    freeToken(&token);
                    setError(ERR_LexFile);
                    return NULL;	
                }
            }
        }
    }
}


