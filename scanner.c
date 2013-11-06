#include "scanner.h"
#include "convert.h"
#include "token_vector.h"
#include "nierr.h"

#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

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
    SS_ExponentSgn,
    SS_DoubleSgn,
    SS_DoubleMantissa,
    SS_ExponentPlusMinus,
    SS_ExponentMantissa,
    SS_StringEscape,
    SS_HexaString,
    SS_HexaString_2,
} ScannerState;

static int32_t charStreamSwitch = 1;
static int32_t lastChar = 0;
static FILE *source = NULL;

// Forward declaration of inner function.
void scannerFillToken(Token *token);

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
            else if (stringCompareS(str, "elseif", 6) == 0) {
                return KTT_Elseif;
            }
            break;
        }
        case 'f': {
            if (stringCompareS(str, "false", 5) == 0) {
                return KTT_False;
            }
            else if (stringCompareS(str, "for", 3) == 0) {
                return KTT_For;
            }
            else if (stringCompareS(str, "function", 8) == 0) {
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

Vector* scannerScanFile(const char *fileName)
{
    scannerOpenFile(fileName);
    Vector* vec = newTokenVector();
    while (1){
        vectorPushDefaultToken(vec);
        Token* token = vectorBack(vec);

        scannerFillToken(token);

        if (getError()){
            freeTokenVector(&vec);
            return NULL;
        }

        if (token->type == STT_EOF)
        {
            vectorShrinkToFit(vec);
            return vec;
        }
    }
}

FILE* scannerOpenFile(const char *fileName) // OPEN FILE AND RETURN POINTER ON IT
{
    if (source != NULL) {
        fclose(source);
        source = NULL;
    }

    source = fopen(fileName, "r");
    if (source != NULL)
        lastChar = getc(source);
    if(source == NULL || lastChar != '<') {
        setError(ERR_LexFile);
        return NULL;
    }
    charStreamSwitch = 0;
    return source;
}

Token* scannerGetToken() // FUNCTION, WHICH RETURNS POINTER ON TOKEN STRUCTURE
{
    Token *token = newToken();

    scannerFillToken(token);
    if (getError()){
        freeToken(&token);
        return NULL;
    }

    return token;
}

void scannerFillToken(Token *token)
{
    ScannerState state = SS_Empty;
    int32_t symbol = 0;
    int32_t hexaString = 0;
    int32_t hexaString_2 = 0;
    int32_t hexaStringBackup = 0;
    int32_t ascii = 0;

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
                    return;
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
                            return;
                        case '{':
                            token->type = STT_LeftCurlyBracket;
                            return;
                        case '}':
                            token->type = STT_RightCurlyBracket;
                            return;
                        case '*':
                            token->type = STT_Asterisk;
                            return;
                        case '+':
                            token->type = STT_Plus;
                            return;
                        case '-':
                            token->type = STT_Minus;
                            return;
                        case '(':
                            token->type = STT_LeftBracket;
                            return;
                        case ')':
                            token->type = STT_RightBracket;
                            return;
                        case ',':
                            token->type = STT_Comma;
                            return;
                        case '.':
                            token->type = STT_Dot;
                            return;
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
                            setError(ERR_LexFile);
                            return;
                    }
                }
                break;
            }
            case SS_Greater: {
                if (symbol == '=') {
                    token->type = STT_GreaterEqual;
                    return;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    token->type = STT_Greater;
                    return;
                }
            }
            case SS_Less: {
                if (symbol == '=') {
                    token->type = STT_LessEqual;
                    return;
                }
                else if (symbol == '?') {
                    state = SS_Php_0;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    token->type = STT_Less;
                    return;
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
                    return;
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
                    setError(ERR_LexFile);
                    return;
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
                    return;
                }
                break;
            }
            case SS_Number: {
                if (symbol >= '0' && symbol <= '9') {
                    state = SS_Number;
                    stringPush(tokenStr, symbol);
                }
                else if (symbol == '.') {
                    state = SS_DoubleSgn;
                    stringPush(tokenStr, symbol);
                }
                else if (symbol == 'e') {
                    state = SS_ExponentSgn;
                    stringPush(tokenStr, symbol);
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    int n = stringToInt(tokenStr);
                    deleteString(tokenStr);
                    token->n = n;
                    token->type = STT_Number;
                    return;
                }
                break;
            }
            case SS_DoubleSgn: {
                if (symbol >= '0' && symbol <= '9') {
                    state = SS_DoubleMantissa;
                    stringPush(tokenStr, symbol);
                }
                else {
                    deleteString(tokenStr);
                    setError(ERR_LexFile);
                    return;
                }
                break;
            }
            case SS_ExponentSgn: {
                if (symbol >= '0' && symbol <= '9') {
                    state = SS_DoubleMantissa;
                    stringPush(tokenStr, symbol);
                }
                else if (symbol == '+' || symbol == '-') {
                    state = SS_ExponentPlusMinus;
                    stringPush(tokenStr, symbol);
                }
                else {
                    deleteString(tokenStr);
                    setError(ERR_LexFile);
                    return;
                }
                break;
            }
            case SS_ExponentPlusMinus: {
                if (symbol >= '0' && symbol <= '9') {
                    state = SS_ExponentMantissa;
                    stringPush(tokenStr, symbol);
                }
                else {
                    deleteString(tokenStr);
                    setError(ERR_LexFile);
                    return;
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
                        return;
                    } else {
                        deleteString(tokenStr);
                        token->type = STT_Keyword;
                        token->keywordType = keywordType;
                        return;
                    }
                }
                break;
            }
            case SS_DoubleMantissa: {
                if (symbol >= '0' && symbol <= '9') {
                    state = SS_DoubleMantissa;
                    stringPush(tokenStr, symbol);
                }
                else if (symbol == 'e' || symbol == 'E') {
                    state = SS_ExponentSgn;
                    stringPush(tokenStr, symbol);
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    double d = stringToDouble(tokenStr);
                    deleteString(tokenStr);
                    token->d = d;
                    token->type = STT_Double;
                    return;
                }
                break;
            }
            case SS_ExponentMantissa: {
                if (symbol >= '0' && symbol <= '9') {
                    state = SS_ExponentMantissa;
                    stringPush(tokenStr, symbol);
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    double d = stringToDouble(tokenStr);
                    deleteString(tokenStr);
                    token->d = d;
                    token->type = STT_Double;
                    return;
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
                    return;
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
                    return;
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
                    return;
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
                    return;
                }
                else {
                    state = SS_Comment;
                }
                break;
            }
            case SS_Equal: {
                if (symbol == '=') {
                    token->type = STT_Equal;
                    return;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    setError(ERR_LexFile);
                    return;
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
                    return;
                }
                break;
            }
            case SS_NotEqual: {
                if (symbol == '=') {
                    token->type = STT_NotEqual;
                    return;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    setError(ERR_LexFile);
                    return;
                }
            }
            case SS_String: {
                if (symbol == '"') {
                    token->type = STT_String;
                    return;
                }
                else if (symbol == EOF) {
                    deleteString(tokenStr);
                    setError(ERR_LexFile);
                    return;
                }
                else {
                    if (symbol > 31 && symbol != '$') {
                        if (symbol == '\\') {
                            state = SS_StringEscape;
                        }
                        else {
                            state = SS_String;
                            stringPush(tokenStr, symbol);
                        }
                    }
                    else {
                        deleteString(tokenStr);
                        setError(ERR_LexFile);
                        return;
                    }
                }
                break;
            }
            case SS_StringEscape: {
                switch (symbol) {
                    case 'x': {
                        state = SS_HexaString;
                        break;
                    }
                    case '$': {
                        state = SS_String;
                        stringPush(tokenStr, symbol);
                        break;
                    }
                    case 'n': {
                        state = SS_String;
                        stringPush(tokenStr, '\n');
                        break;
                    }
                    case 't': {
                        state = SS_String;
                        stringPush(tokenStr, '\t');
                        break;
                    }
                    case '\"': {
                        state = SS_String;
                        stringPush(tokenStr, '\"');
                        break;
                    }
                    case '\\': {
                        state = SS_String;
                        stringPush(tokenStr, '\\');
                        break;
                    }
                    default: {
                        state = SS_String;
                        stringPush(tokenStr, '\\');
                        stringPush(tokenStr, symbol);
                        break;
                    }
                }
                break;
            }
            case SS_HexaString_2: {
                if (symbol >= '0' && symbol <= '9') {
                    hexaString_2 = symbol - '0';
                    ascii = 16 * hexaString + hexaString_2;
                    stringPush(tokenStr, ascii);
                    state = SS_String;
                }
                else if (symbol >= 'A' && symbol <= 'F') {
                    hexaString_2 = symbol - 'A' + 10;
                    ascii = 16 * hexaString + hexaString_2;
                    stringPush(tokenStr, ascii);
                    state = SS_String;
                }
                else if (symbol >= 'a' && symbol <= 'f') {
                    hexaString_2 = symbol - 'a' + 10;
                    ascii = 16 * hexaString + hexaString_2;
                    stringPush(tokenStr, ascii);
                    state = SS_String;
                }
                else {
                    state = SS_String;
                    stringPush(tokenStr, '\\');
                    stringPush(tokenStr, 'x');
                    stringPush(tokenStr, hexaStringBackup);
                    stringPush(tokenStr, symbol);
                }
                break;
            }
            case SS_HexaString: {
                if (symbol >= '0' && symbol <= '9') {
                    hexaString = symbol - '0';
                    state = SS_HexaString_2;
                }
                else if (symbol >= 'A' && symbol <= 'F') {
                    hexaString = symbol - 'A' + 10;
                    state = SS_HexaString_2;
                }
                else if (symbol >= 'a' && symbol <= 'f') {
                    hexaString = symbol - 'a' + 10;
                    state = SS_HexaString_2;
                }
                else {
                    state = SS_String;
                    stringPush(tokenStr, '\\');
                    stringPush(tokenStr, 'x');
                    stringPush(tokenStr, symbol);
                }
                hexaStringBackup = symbol;
                break;
            }
            case SS_Php_0: {
                if (symbol == 'p') {
                    state = SS_Php_1;
                }
                else {
                    setError(ERR_LexFile);
                    return;
                }
                break;
            }
            case SS_Php_1: {
                if (symbol == 'h') {
                    state = SS_Php_2;
                }
                else {
                    setError(ERR_LexFile);
                    return;
                }
                break;
            }
            case SS_Php_2: {
                if (symbol == 'p') {
                    state = SS_Php_3;
                }
                else {
                    setError(ERR_LexFile);
                    return;
                }
                break;
            }
            case SS_Php_3: {
                if (isspace(symbol)) {
                    token->type = STT_Php;
                    return;
                }
                else {
                    setError(ERR_LexFile);
                    return;
                }
            }
        }
    }
}



