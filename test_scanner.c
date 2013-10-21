#include "test.h"
#include "scanner.h"
#include "nierr.h"

char filePath[32];
Token *token;

void generateFilePath()
{
    sprintf(filePath, "/tmp/ini-test.%d", getpid());
}

uint8_t overwriteFile(const char *fileName, const char *content)
{
    if (!fileName)
        return 0;

    FILE *file = fopen(fileName, "w+");
    if (!file)
        return 0;

    fprintf(file, "%s", content);
    scannerReset();
    scannerOpenFile(filePath);
    fclose(file);
    return 1;
}

TEST_SUITE_START(ScannerTests);

generateFilePath();

SHOULD_EQUAL("openFile - not existing file", scannerOpenFile(filePath), NULL);

overwriteFile(filePath, "");
SHOULD_NOT_EQUAL("openFile - existing file", scannerOpenFile(filePath), NULL);

token = scannerGetToken();
SHOULD_EQUAL("GetToken - empty file - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, ";");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - semicolon", token->type, STT_Semicolon);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - semicolon - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "{");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - left curly bracket", token->type, STT_LeftCurlyBracket);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - left curly bracket - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "*");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - asterisk", token->type, STT_Asterisk);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - asterisk - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "}");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - right curly bracket", token->type, STT_RightCurlyBracket);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - right curly bracket - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "+");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - plus", token->type, STT_Plus);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - plus - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "-");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - minus", token->type, STT_Minus);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - minus - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, ")");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - right bracket", token->type, STT_RightBracket);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - right bracket - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "(");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - left bracket", token->type, STT_LeftBracket);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - left bracket - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "id");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - identifier", token->type, STT_Identifier);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - identifier - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "$var");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - $var", token->type, STT_Variable);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - $var - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "1337");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - number", token->type, STT_Number);
SHOULD_EQUAL("GetToken - number - Value", token->n, 1337);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - number - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "function");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - keyword (function)", token->type, STT_Keyword);
SHOULD_EQUAL("GetToken - keyword (function) - keyword type", token->keywordType, KTT_Function);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - keyword (function) - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, ",");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - comma", token->type, STT_Comma);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - comma - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "=");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - assignment", token->type, STT_Assignment);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - assignment - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, ">");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - greater", token->type, STT_Greater);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - greater - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, ">=");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - greater equal", token->type, STT_GreaterEqual);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - greater equal - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "<");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - less", token->type, STT_Less);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - less - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "<=");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - less equal", token->type, STT_LessEqual);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - less equal - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "===");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - equal", token->type, STT_Equal);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - equal - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "!==");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - not equal", token->type, STT_NotEqual);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - not equal - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "\"string\"");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - string", token->type, STT_String);
SHOULD_EQUAL_STR("GetToken - string - string data", token->str.data, "string");
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - string - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "12.03");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - double", token->type, STT_Double);
SHOULD_EQUAL("GetToken - double - Value", token->d, 12.03);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - double - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "12.03E+04");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - double exponent", token->type, STT_Double);
SHOULD_EQUAL("GetToken - double exponent - Value", token->d, 12.03E+04);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - double exponent - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "/");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - divide", token->type, STT_Divide);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - divide - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "<?php");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - <?php", token->type, STT_Php);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - <?php - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "var 123;");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - multitoken - id", token->type, STT_Identifier);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - multitoken - number", token->type, STT_Number);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - multitoken - semicolon", token->type, STT_Semicolon);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - multitoken - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "12.");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - lex error double", token, NULL);
freeToken(&token);

overwriteFile(filePath, ":");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - Colon", token, NULL);
freeToken(&token);

overwriteFile(filePath, "$");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - lex error $", token, NULL);
freeToken(&token);

overwriteFile(filePath, "$s.$s");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - multitoken - concat - operand1", token->type, STT_Variable);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - multitoken - concat - operator", token->type, STT_Dot);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - multitoken - concat - operand2", token->type, STT_Variable);
freeToken(&token);

overwriteFile(filePath, "$var //comment $comment");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - multitoken - comment - var", token->type, STT_Variable);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - multitoken - comment - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "$var /* comment \n comment2 */ $var2");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - multitoken - multiline comment - var1", token->type, STT_Variable);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - multitoken - multiline comment - var2", token->type, STT_Variable);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - multitoken - multiline comment - EOF", token->type, STT_EOF);
freeToken(&token);

overwriteFile(filePath, "==");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - lex error double equal", token, NULL);
freeToken(&token);

overwriteFile(filePath, "if else elseif continue break while for true false return function null");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - keywordType - If", token->keywordType, KTT_If);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - keywordType - Else", token->keywordType, KTT_Else);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - keywordType - ElseIf", token->keywordType, KTT_Elseif);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - keywordType - Continue", token->keywordType, KTT_Continue);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - keywordType - Break", token->keywordType, KTT_Break);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - keywordType - While", token->keywordType, KTT_While);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - keywordType - For", token->keywordType, KTT_For);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - keywordType - True", token->keywordType, KTT_True);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - keywordType - False", token->keywordType, KTT_False);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - keywordType - Return", token->keywordType, KTT_Return);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - keywordType - Function", token->keywordType, KTT_Function);
freeToken(&token);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - keywordType - Null", token->keywordType, KTT_Null);
freeToken(&token);

// @todo need tests for symbol table.

scannerReset();

TEST_SUITE_END
