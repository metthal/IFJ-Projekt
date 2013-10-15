#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include "scanner.h"

int32_t charStreamSwitch = 1;
int32_t lastChar;
FILE *source;

ScannerTokenType scannerGetToken()
{
    // TODO ScannerTokenType token = STT_Empty;
    ScannerTokenState tokenState = STS_NOT_FINISHED;
    ScannerState state = SS_Empty;
    int32_t symbol;

    while (tokenState == STS_NOT_FINISHED)
    {
        if (charStreamSwitch) {
            if ((symbol = getc(source)) == EOF) {
                return STT_EOF;
            }
        }
        else {
            symbol = lastChar;
            charStreamSwitch=1;
        }
        switch (state) {
            case SS_Empty : {
                if ((symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z')) {
                    state = SS_Identifier;
                }
                else if (symbol >= '0' && symbol <= '9')  {
                    state = SS_Number;
                }
                else if (isspace(symbol))
                {
                    ;
                }
                else {
                    switch (symbol) {
                        case '"' : {
                            state = SS_String;
                            break;
                        }
                        case '/' : {
                            state = SS_Divide;
                            break;
                        }
                        case '!' : {
                            state = SS_Exclamation;
                            break;
                        }
                        case ';' : {
                            return STT_Semicolon;
                        }
                        case '{' : {
                            return STT_LeftCurlyBracket;
                        }
                        case '}' : {
                            return STT_RightCurlyBracket;
                        }
                        case '*' : {
                            return STT_Asterisk;
                        }
                        case '+' : {
                            return STT_Plus;
                        }
                        case '-' : {
                            return STT_Minus;
                        }
                        case '(' : {
                            return STT_LeftBracket;
                        }
                        case ')' : {
                            return STT_RightBracket;
                        }
                        case ',' : {
                            return STT_Comma;
                        }
                        case ':' : {
                            return STT_Colon;
                        }
                        case '>' : {
                            state = SS_Greater;
                            break;
                        }
                        case '<' : {
                            state = SS_Less;
                            break;
                        }
                        case '=' : {
                            state = SS_Assigment;
                            break;
                        }
                        case '$' : {
                            state = SS_Dollar;
                            break;
                        }
                    }
                }
                break;
            }
            case SS_Greater : {
                if (symbol == '=') {
                    return STT_GreaterEqual;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    return STT_Greater;
                }
            }
            case SS_Less : {
                if (symbol == '=') {
                    return STT_LessEqual;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    return STT_Less;
                }
            }
            case SS_Assigment : {
                if (symbol == '=') {
                    state = SS_Equal;
                    break;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    return STT_Assigment;
                }
            }
            case SS_Dollar : {
                if ((symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z') || (symbol == '_')) {
                    state = SS_Variable;
                    // STRING PUSH
                    break;
                }
                else {
                    return STT_LexError;// LEX ERROR ???
                }
            }
            case SS_Variable : {
                if ((symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z') || (symbol == '_') ||
                     (symbol >= '0' && symbol <= '9')) {
                    // STRING PUSH
                    state = SS_Variable; // STILL THE SAME STATE
                    break;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    return STT_Variable; // PLUS RETURN STRING SOMEWHERE
                }
            }
            case SS_Number : {
                if (symbol >= '0' && symbol <= '9') {
                    state = SS_Number;
                    // PUSH STRING
                    break;
                }
                else if (symbol == '.') {
                    state = SS_Double;
                }
                // ELSE IF EXPONENT
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    return STT_Number;
                }
            }
            case SS_Identifier : {
                if ((symbol == '_') || (symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z') || (symbol >= '0' && symbol <= '9')) {
                    state = SS_Identifier;
                    break;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    return STT_Identifier;
                }
            }
            case SS_Double : {
                if (symbol >= '0' && symbol <= '9') {
                    state = SS_Double;
                    // PUSH STRING
                    break;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    return STT_Double;
                }
            }
            case SS_Divide : {
                if (symbol == '*') {
                    state = SS_BlockComment;
                    break;
                }
                else if (symbol == '/') {
                    state = SS_Comment;
                    break;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    return STT_Divide;
                }
            }
            case SS_BlockComment : {
                if (symbol == '*') {
                    state = SS_BlockCommentFinish;
                    break;
                }
                else {
                    state = SS_BlockComment;
                    break;
                }
            }
            case SS_BlockCommentFinish : {
                if (symbol == '/') {
                    state = SS_Empty;
                    break;
                }
                else {
                    state = SS_BlockComment;
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    break;
                }
            }
            case SS_Comment : {
                if (symbol == '\n') {
                    state = SS_Empty;
                }
                else {
                    state = SS_Comment;
                }
                break;
            }
            case SS_Equal : {
                if (symbol == '=') {
                    return STT_Equal;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    return STT_LexError;
                }
            }
            case SS_Exclamation : {
                if (symbol == '=') {
                    state = SS_NotEqual;
                    break;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    return STT_LexError;
                }
            }
            case SS_NotEqual : {
                if (symbol == '=') {
                    return STT_NotEqual;
                }
                else {
                    lastChar = symbol;
                    charStreamSwitch = 0;
                    return STT_LexError;
                }
            }
            case SS_String : {
                if (symbol == '"') {
                    return STT_String;
                }
                else {
                    state = SS_String;
                    // CONTENT OF THE STRING + NEW AUTOMAT
                }
            }
        }

    }

    return STT_LexError
}


