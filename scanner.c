#include "scanner.h"
#include "convert.h"
#include "token_vector.h"
#include "nierr.h"

#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#define MIN_STRING_CHAR_ASCII   32

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
    SS_DoubleDecPoint,
    SS_DoubleDecPart,
    SS_DoubleExponent,
    SS_DoubleExpSign,
    SS_DoubleExpPart,
    SS_StringEscape,
    SS_StringEscapeHex0,
    SS_StringEscapeHex1,
    SS_Php_0,
    SS_Php_1,
    SS_Php_2,
    SS_Php_3,
    SS_And,
    SS_Or
} ScannerState;

static int32_t charStreamSwitch = 1;
static int32_t lastChar = 0;
static FILE *source = NULL;

// Forward declaration of inner function.
void scannerFillToken(Token *token);

KeywordTokenType strToKeyword(String *str)
{
    switch (str->data[0]) {
        case 'a': {
            if (stringCompareS(str, "and", 3) == 0) {
                return KTT_And;
            }
            break;
        }
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
        case 'o': {
            if (stringCompareS(str, "or", 2) == 0) {
                return KTT_Or;
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
    if(getError())
        return NULL;

    Vector* vec = newTokenVector();
    while (1) {
        vectorPushDefaultToken(vec);
        Token *token = vectorBack(vec);

        scannerFillToken(token);

        if (getError()) {
            freeTokenVector(&vec);
            fclose(source);
            source = NULL;
            return NULL;
        }

        if (token->type == STT_EOF)
        {
            fclose(source);
            source = NULL;
            vectorShrinkToFit(vec);
            return vec;
        }
    }
    return NULL;
}

FILE* scannerOpenFile(const char *fileName) // OPEN FILE AND RETURN POINTER ON IT
{
    if (source != NULL) {
        fclose(source);
        source = NULL;
    }

    source = fopen(fileName, "r");
    if(source == NULL) {
        setError(ERR_LexFile);
        return NULL;
    }
    else if ((lastChar = getc(source)) != '<') {
        // Syntax error according to forums
        setError(ERR_Syntax);
        fclose(source);
        source = NULL;
        return NULL;
    }

    charStreamSwitch = 0;
    return source;
}

Token* scannerGetToken() // FUNCTION, WHICH RETURNS POINTER ON TOKEN STRUCTURE
{
    Token *token = newToken();

    scannerFillToken(token);
    if (getError()) {
        freeToken(&token);
        fclose(source);
        source = NULL;
        return NULL;
    }

    if (token->type == STT_EOF) {
        fclose(source);
        source = NULL;
    }

    return token;
}

void scannerFillToken(Token *token)
{
    ScannerState state = SS_Empty;
    int32_t symbol = 0;
    char hexCode = 0;
    char hexCodeBackup = 0;

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
                if ((symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z') || (symbol == '_')) {
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
                            token->type = STT_Multiply;
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
                        case '&':
                            state = SS_And;
                            break;
                        case '|':
                            state = SS_Or;
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
                    state = SS_DoubleDecPoint;
                    stringPush(tokenStr, symbol);
                }
                else if ((symbol == 'e') || (symbol == 'E')) {
                    state = SS_DoubleExponent;
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
            case SS_DoubleDecPoint: {
                if (symbol >= '0' && symbol <= '9') {
                    state = SS_DoubleDecPart;
                    stringPush(tokenStr, symbol);
                }
                else {
                    deleteString(tokenStr);
                    setError(ERR_LexFile);
                    return;
                }
                break;
            }
            case SS_DoubleExponent: {
                if (symbol >= '0' && symbol <= '9') {
                    state = SS_DoubleExpPart;
                    stringPush(tokenStr, symbol);
                }
                else if ((symbol == '+') || (symbol == '-')) {
                    state = SS_DoubleExpSign;
                    stringPush(tokenStr, symbol);
                }
                else {
                    deleteString(tokenStr);
                    setError(ERR_LexFile);
                    return;
                }
                break;
            }
            case SS_DoubleExpSign: {
                if (symbol >= '0' && symbol <= '9') {
                    state = SS_DoubleExpPart;
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
                    }
                    else {
                        deleteString(tokenStr);
                        if (keywordType == KTT_True) {
                            token->type = STT_Bool;
                            token->n = 1;
                        }
                        else if (keywordType == KTT_False) {
                            token->type = STT_Bool;
                            token->n = 0;
                        }
                        else if (keywordType == KTT_Null) {
                            token->type = STT_Null;
                        }
                        else if (keywordType == KTT_And) {
                            token->type = STT_And;
                        }
                        else if (keywordType == KTT_Or) {
                            token->type = STT_Or;
                        }
                        else {
                            token->type = STT_Keyword;
                            token->keywordType = keywordType;
                        }
                        return;
                    }
                }
                break;
            }
            case SS_DoubleDecPart: {
                if (symbol >= '0' && symbol <= '9') {
                    state = SS_DoubleDecPart;
                    stringPush(tokenStr, symbol);
                }
                else if (symbol == 'e' || symbol == 'E') {
                    state = SS_DoubleExponent;
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
            case SS_DoubleExpPart: {
                if (symbol >= '0' && symbol <= '9') {
                    state = SS_DoubleExpPart;
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
                    token->type = STT_Not;
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
                    if (symbol >= MIN_STRING_CHAR_ASCII && symbol != '$') {
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
                        state = SS_StringEscapeHex0;
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
            case SS_StringEscapeHex0: {
                if (symbol >= '0' && symbol <= '9') {
                    hexCode = symbol - '0';
                    state = SS_StringEscapeHex1;
                }
                else if (symbol >= 'A' && symbol <= 'F') {
                    hexCode = symbol - 'A' + 10;
                    state = SS_StringEscapeHex1;
                }
                else if (symbol >= 'a' && symbol <= 'f') {
                    hexCode = symbol - 'a' + 10;
                    state = SS_StringEscapeHex1;
                }
                else {
                    stringPush(tokenStr, '\\');
                    stringPush(tokenStr, 'x');
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    state = SS_String;
                }
                hexCodeBackup = symbol;
                break;
            }
            case SS_StringEscapeHex1: {
                if (symbol >= '0' && symbol <= '9') {
                    hexCode = (hexCode << 4) + (symbol - '0');
                    stringPush(tokenStr, hexCode);
                    state = SS_String;
                }
                else if (symbol >= 'A' && symbol <= 'F') {
                    hexCode = (hexCode << 4) + (symbol - 'A' + 10);
                    stringPush(tokenStr, hexCode);
                    state = SS_String;
                }
                else if (symbol >= 'a' && symbol <= 'f') {
                    hexCode = (hexCode << 4) + (symbol - 'a' + 10);
                    stringPush(tokenStr, hexCode);
                    state = SS_String;
                }
                else {
                    stringPush(tokenStr, '\\');
                    stringPush(tokenStr, 'x');
                    stringPush(tokenStr, hexCodeBackup);
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    state = SS_String;
                }
                break;
            }
            case SS_Php_0: {
                if (symbol == 'p') {
                    state = SS_Php_1;
                }
                else {
                    setError(ERR_Syntax);
                    return;
                }
                break;
            }
            case SS_Php_1: {
                if (symbol == 'h') {
                    state = SS_Php_2;
                }
                else {
                    setError(ERR_Syntax);
                    return;
                }
                break;
            }
            case SS_Php_2: {
                if (symbol == 'p') {
                    state = SS_Php_3;
                }
                else {
                    setError(ERR_Syntax);
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
                    setError(ERR_Syntax);
                    return;
                }
            }
            case SS_And:
                if (symbol != '&') {
                    setError(ERR_LexFile);
                    return;
                }

                token->type = STT_And;
                return;
            case SS_Or:
                if (symbol != '|') {
                    setError(ERR_LexFile);
                    return;
                }

                token->type = STT_Or;
                return;
        }
    }
}
