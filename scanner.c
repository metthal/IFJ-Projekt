#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include "scanner.h"
#include "string.h"
#include "convert.h"
#include "nierr.h"

int32_t charStreamSwitch = 1;
int32_t lastChar;
FILE *source;

void resetScanner()
{
    charStreamSwitch = 1;
    lastChar = 0;
    if (source) {
        fclose(source);
        source = NULL;
    }
}

void checkKeyword(String *str, Token *token)
{
    switch (str->data[0]) {
        case 'e': {
            if (stringCompareS(str, "else", 4) == 0) {
                token->type = STT_Keyword;
                token->keywordType = KTT_Else;
            }
            else if (stringCompareS(str, "elseif", 6) == 0){
                token->type = STT_Keyword;
                token->keywordType = KTT_Elseif;
            }
            else {
                token->type = STT_Identifier;
            }
            break;
        }
        case 'f': {
            if (stringCompareS(str, "false", 5) == 0) {
                token->type = STT_Keyword;
                token->keywordType = KTT_False;
            }
            else if (stringCompareS(str, "for", 3) == 0){
                token->type = STT_Keyword;
                token->keywordType = KTT_For;
            }
            else if (stringCompareS(str, "function", 8) == 0){
                token->type = STT_Keyword;
                token->keywordType = KTT_Function;
            }
            else {
                token->type = STT_Identifier;
            }
            break;
        }
        case 'i': {
            if (stringCompareS(str, "if", 2) == 0) {
                token->type = STT_Keyword;
                token->keywordType = KTT_If;
            }
            else {
                token->type = STT_Identifier;
            }
            break;
        }
        case 'n': {
            if (stringCompareS(str, "null", 4) == 0) {
                token->type = STT_Keyword;
                token->keywordType = KTT_Null;
            }
            else {
                token->type = STT_Identifier;
            }
            break;
        }
        case 'r': {
            if (stringCompareS(str, "return", 6) == 0) {
                token->type = STT_Keyword;
                token->keywordType = KTT_Return;
            }
            else {
                token->type = STT_Identifier;
            }
            break;
        }
        case 't': {
            if (stringCompareS(str, "true", 4) == 0) {
                token->type = STT_Keyword;
                token->keywordType = KTT_True;
            }
            else {
                token->type = STT_Identifier;
            }
            break;
        }
        case 'w': {
            if (stringCompareS(str, "while", 5) == 0) {
                token->type = STT_Keyword;
                token->keywordType = KTT_While;
            }
            else {
                token->type = STT_Identifier;
            }
            break;
        }
        default:
            token->type = STT_Identifier;
            break;
    }
}

FILE* openFile(const char *fileName) // OPEN FILE AND RETURN POINTER ON IT
{
    if (source) {
        fclose(source);
        source = NULL;
    }

    source = fopen(fileName, "r");
    return source;
}

Token* newToken() // FUNCTION CREATES NEWTOKEN STRUCTURE
{
    Token* tmp = malloc(sizeof(Token));
    return tmp;
}

Token* scannerGetToken() // FUNCTION, WHICH RETURNS POINTER ON TOKEN STRUCTURE
{
    Token *token = newToken();
    // TODO ScannerTokenType token = STT_Empty;
    ScannerTokenState tokenState = STS_NOT_FINISHED;
    ScannerState state = SS_Empty;
    int32_t symbol;
    int32_t firsTimeSwitch = 1;

    String *tmpStrPtr = &(token->str);
    initString(tmpStrPtr);

    String *strDigits = newString();

    while (tokenState == STS_NOT_FINISHED)
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
                    stringPush(tmpStrPtr,symbol);
                }
                else if (symbol >= '0' && symbol <= '9') {
                    stringPush(strDigits,symbol);
                    state = SS_Number;
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
                        case ':':
                            token->type = STT_Colon;
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
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    token->type = STT_Less;
                    return token;
                }
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
                    stringPush(tmpStrPtr, symbol);
                    break;
                }
                else {
                    setError(ERR_LexFile);
                    return NULL;
                }
                break;
            }
            case SS_Variable: {
                if ((symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z') || (symbol == '_') ||
                     (symbol >= '0' && symbol <= '9')) {
                    stringPush(tmpStrPtr, symbol);
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
                    stringPush(strDigits, symbol);
                }
                else if (symbol == '.') {
                    state = SS_Double;
                    stringPush(strDigits, symbol);
                }
                // TODO ELSE IF EXPONENT
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    token->n = stringToInt(strDigits);
                    token->type = STT_Number;
                    return token;
                }
                break;
            }
            case SS_Identifier: {
                if ((symbol == '_') || (symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z') || (symbol >= '0' && symbol <= '9')) {
                    stringPush(tmpStrPtr, symbol);
                    state = SS_Identifier;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    checkKeyword(tmpStrPtr, token); // FUNCTION CHECKS IF IDENTIFIER, WHICH HAS BEEN FOUND AINT A RESERVED ( KEYWORD ) WORD
                    return token;
                }
                break;
            }
            case SS_Double: {
                if (symbol >= '0' && symbol <= '9') {
                    state = SS_Double;
                    stringPush(strDigits, symbol);
                    firsTimeSwitch = 0;
                }
                else if ((symbol == EOF) && (firsTimeSwitch)) {
                    setError(ERR_LexFile);
                    return NULL;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    token->d = stringToDouble(strDigits);
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
                    setError(ERR_LexFile);
                    return NULL;
                }
                else {
                    state = SS_String;
                    stringPush(tmpStrPtr, symbol);
                    // TODO = AUTOMAT
                }
                break;
            }
        }
    }

    setError(ERR_LexFile);
    return NULL;
}


