#include "test.h"
#include "scanner.h"
#include "nierr.h"

char filePath[32];
Token *token;

Token* nextToken()
{
    if (token)
        freeToken(&token);

    return scannerGetToken();
}

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

    fprintf(file, "%s\n%s", "<?php", content);
    fclose(file);

    scannerReset();
    scannerOpenFile(filePath);

    // Skip php token
    token = nextToken();

    return 1;
}

TEST_SUITE_START(ScannerTests);

generateFilePath();
token = NULL;

SHOULD_EQUAL("openFile - not existing file", scannerOpenFile(filePath), NULL);
clearError();

overwriteFile(filePath, "");
SHOULD_NOT_EQUAL("openFile - existing file", scannerOpenFile(filePath), NULL);

overwriteFile(filePath, "");
token = nextToken();
SHOULD_EQUAL("GetToken - empty file - EOF", token->type, STT_EOF);

overwriteFile(filePath, ";");
token = nextToken();
SHOULD_EQUAL("GetToken - semicolon", token->type, STT_Semicolon);
token = nextToken();
SHOULD_EQUAL("GetToken - semicolon - EOF", token->type, STT_EOF);

overwriteFile(filePath, "{");
token = nextToken();
SHOULD_EQUAL("GetToken - left curly bracket", token->type, STT_LeftCurlyBracket);
token = nextToken();
SHOULD_EQUAL("GetToken - left curly bracket - EOF", token->type, STT_EOF);

overwriteFile(filePath, "*");
token = nextToken();
SHOULD_EQUAL("GetToken - multiply", token->type, STT_Multiply);
token = nextToken();
SHOULD_EQUAL("GetToken - multiply - EOF", token->type, STT_EOF);

overwriteFile(filePath, "}");
token = nextToken();
SHOULD_EQUAL("GetToken - right curly bracket", token->type, STT_RightCurlyBracket);
token = nextToken();
SHOULD_EQUAL("GetToken - right curly bracket - EOF", token->type, STT_EOF);

overwriteFile(filePath, "+");
token = nextToken();
SHOULD_EQUAL("GetToken - plus", token->type, STT_Plus);
token = nextToken();
SHOULD_EQUAL("GetToken - plus - EOF", token->type, STT_EOF);

overwriteFile(filePath, "-");
token = nextToken();
SHOULD_EQUAL("GetToken - minus", token->type, STT_Minus);
token = nextToken();
SHOULD_EQUAL("GetToken - minus - EOF", token->type, STT_EOF);

overwriteFile(filePath, ")");
token = nextToken();
SHOULD_EQUAL("GetToken - right bracket", token->type, STT_RightBracket);
token = nextToken();
SHOULD_EQUAL("GetToken - right bracket - EOF", token->type, STT_EOF);

overwriteFile(filePath, "(");
token = nextToken();
SHOULD_EQUAL("GetToken - left bracket", token->type, STT_LeftBracket);
token = nextToken();
SHOULD_EQUAL("GetToken - left bracket - EOF", token->type, STT_EOF);

overwriteFile(filePath, "id");
token = nextToken();
SHOULD_EQUAL("GetToken - identifier", token->type, STT_Identifier);
token = nextToken();
SHOULD_EQUAL("GetToken - identifier - EOF", token->type, STT_EOF);

overwriteFile(filePath, "_id");
token = nextToken();
SHOULD_EQUAL("GetToken - identifier underscore", token->type, STT_Identifier);
token = nextToken();
SHOULD_EQUAL("GetToken - identifier underscore - EOF", token->type, STT_EOF);

overwriteFile(filePath, "$var");
token = nextToken();
SHOULD_EQUAL("GetToken - $var", token->type, STT_Variable);
token = nextToken();
SHOULD_EQUAL("GetToken - $var - EOF", token->type, STT_EOF);

overwriteFile(filePath, "$if");
token = nextToken();
SHOULD_EQUAL("GetToken - $if - variable", token->type, STT_Variable);
token = nextToken();
SHOULD_EQUAL("GetToken - $if - EOF", token->type, STT_EOF);

overwriteFile(filePath, "$_var");
token = nextToken();
SHOULD_EQUAL("GetToken - $_var - variable", token->type, STT_Variable);
token = nextToken();
SHOULD_EQUAL("GetToken - $_var - EOF", token->type, STT_EOF);

overwriteFile(filePath, "1337");
token = nextToken();
SHOULD_EQUAL("GetToken - number", token->type, STT_Number);
SHOULD_EQUAL("GetToken - number - Value", token->n, 1337);
token = nextToken();
SHOULD_EQUAL("GetToken - number - EOF", token->type, STT_EOF);

overwriteFile(filePath, "function");
token = nextToken();
SHOULD_EQUAL("GetToken - keyword (function)", token->type, STT_Keyword);
SHOULD_EQUAL("GetToken - keyword (function) - keyword type", token->keywordType, KTT_Function);
token = nextToken();
SHOULD_EQUAL("GetToken - keyword (function) - EOF", token->type, STT_EOF);

overwriteFile(filePath, ",");
token = nextToken();
SHOULD_EQUAL("GetToken - comma", token->type, STT_Comma);
token = nextToken();
SHOULD_EQUAL("GetToken - comma - EOF", token->type, STT_EOF);

overwriteFile(filePath, "=");
token = nextToken();
SHOULD_EQUAL("GetToken - assignment", token->type, STT_Assignment);
token = nextToken();
SHOULD_EQUAL("GetToken - assignment - EOF", token->type, STT_EOF);

overwriteFile(filePath, ">");
token = nextToken();
SHOULD_EQUAL("GetToken - greater", token->type, STT_Greater);
token = nextToken();
SHOULD_EQUAL("GetToken - greater - EOF", token->type, STT_EOF);

overwriteFile(filePath, ">=");
token = nextToken();
SHOULD_EQUAL("GetToken - greater equal", token->type, STT_GreaterEqual);
token = nextToken();
SHOULD_EQUAL("GetToken - greater equal - EOF", token->type, STT_EOF);

overwriteFile(filePath, "<");
token = nextToken();
SHOULD_EQUAL("GetToken - less", token->type, STT_Less);
token = nextToken();
SHOULD_EQUAL("GetToken - less - EOF", token->type, STT_EOF);

overwriteFile(filePath, "<=");
token = nextToken();
SHOULD_EQUAL("GetToken - less equal", token->type, STT_LessEqual);
token = nextToken();
SHOULD_EQUAL("GetToken - less equal - EOF", token->type, STT_EOF);

overwriteFile(filePath, "===");
token = nextToken();
SHOULD_EQUAL("GetToken - equal", token->type, STT_Equal);
token = nextToken();
SHOULD_EQUAL("GetToken - equal - EOF", token->type, STT_EOF);

overwriteFile(filePath, "!==");
token = nextToken();
SHOULD_EQUAL("GetToken - not equal", token->type, STT_NotEqual);
token = nextToken();
SHOULD_EQUAL("GetToken - not equal - EOF", token->type, STT_EOF);

overwriteFile(filePath, "\"string\"");
token = nextToken();
SHOULD_EQUAL("GetToken - string", token->type, STT_String);
SHOULD_EQUAL_STR("GetToken - string - string data", token->str.data, "string");
token = nextToken();
SHOULD_EQUAL("GetToken - string - EOF", token->type, STT_EOF);

overwriteFile(filePath, "\"\\x\"");
token = nextToken();
SHOULD_EQUAL("GetToken - string", token->type, STT_String);
SHOULD_EQUAL_STR("GetToken - string - string data", token->str.data, "\\x");
token = nextToken();
SHOULD_EQUAL("GetToken - string - EOF", token->type, STT_EOF);

overwriteFile(filePath, "\"\\x\\\"\"");
token = nextToken();
SHOULD_EQUAL("GetToken - string", token->type, STT_String);
SHOULD_EQUAL_STR("GetToken - string - string data", token->str.data, "\\x\"");
token = nextToken();
SHOULD_EQUAL("GetToken - string - EOF", token->type, STT_EOF);

overwriteFile(filePath, "\"\\xa\\\"\"");
token = nextToken();
SHOULD_EQUAL("GetToken - string", token->type, STT_String);
SHOULD_EQUAL_STR("GetToken - string - string data", token->str.data, "\\xa\"");
token = nextToken();
SHOULD_EQUAL("GetToken - string - EOF", token->type, STT_EOF);

// "Ahoj
// $svete!` \a\'\r\f\b\0\v"
overwriteFile(filePath, "\"\\\"Ahoj\\n\\$vete\\x21` \\a\\'\\r\\f\\b\\0\\v\\\"\"");
token = nextToken();
SHOULD_EQUAL("GetToken - string - escape sequences", token->type, STT_String);
SHOULD_EQUAL_STR("GetToken - string - escape sequences - data", token->str.data, "\"Ahoj\n$vete!` \\a\\'\\r\\f\\b\\0\\v\"");

overwriteFile(filePath, "12.03");
token = nextToken();
SHOULD_EQUAL("GetToken - double", token->type, STT_Double);
SHOULD_EQUAL("GetToken - double - Value", token->d, 12.03);
token = nextToken();
SHOULD_EQUAL("GetToken - double - EOF", token->type, STT_EOF);

overwriteFile(filePath, "12E-1");
token = nextToken();
SHOULD_EQUAL("GetToken - double", token->type, STT_Double);
SHOULD_EQUAL("GetToken - double - Value", token->d, 12E-1);
token = nextToken();
SHOULD_EQUAL("GetToken - double - EOF", token->type, STT_EOF);

overwriteFile(filePath, "12.03E+04");
token = nextToken();
SHOULD_EQUAL("GetToken - double exponent", token->type, STT_Double);
SHOULD_EQUAL("GetToken - double exponent - Value", token->d, 12.03E+04);
token = nextToken();
SHOULD_EQUAL("GetToken - double exponent - EOF", token->type, STT_EOF);

overwriteFile(filePath, "/");
token = nextToken();
SHOULD_EQUAL("GetToken - divide", token->type, STT_Divide);
token = nextToken();
SHOULD_EQUAL("GetToken - divide - EOF", token->type, STT_EOF);

overwriteFile(filePath, "var 123;");
token = nextToken();
SHOULD_EQUAL("GetToken - multitoken - id", token->type, STT_Identifier);
token = nextToken();
SHOULD_EQUAL("GetToken - multitoken - number", token->type, STT_Number);
token = nextToken();
SHOULD_EQUAL("GetToken - multitoken - semicolon", token->type, STT_Semicolon);
token = nextToken();
SHOULD_EQUAL("GetToken - multitoken - EOF", token->type, STT_EOF);

overwriteFile(filePath, "12.");
token = nextToken();
SHOULD_EQUAL("GetToken - lex error double", token, NULL);
clearError();

overwriteFile(filePath, "12.e03");
token = nextToken();
SHOULD_EQUAL("GetToken - lex error double", token, NULL);
clearError();

overwriteFile(filePath, "12.3e-");
token = nextToken();
SHOULD_EQUAL("GetToken - lex error double", token, NULL);
clearError();

overwriteFile(filePath, ":");
token = nextToken();
SHOULD_EQUAL("GetToken - lex error Colon", token, NULL);
clearError();

overwriteFile(filePath, "$");
token = nextToken();
SHOULD_EQUAL("GetToken - lex error $", token, NULL);
clearError();

overwriteFile(filePath, "\"string");
token = nextToken();
SHOULD_EQUAL("GetToken - unexpected end of string - lex error", token, NULL);
clearError();

overwriteFile(filePath, "$s.$s");
token = nextToken();
SHOULD_EQUAL("GetToken - multitoken - concat - operand1", token->type, STT_Variable);
token = nextToken();
SHOULD_EQUAL("GetToken - multitoken - concat - operator", token->type, STT_Dot);
token = nextToken();
SHOULD_EQUAL("GetToken - multitoken - concat - operand2", token->type, STT_Variable);

overwriteFile(filePath, "$var //comment $comment");
token = nextToken();
SHOULD_EQUAL("GetToken - multitoken - comment - var", token->type, STT_Variable);
token = nextToken();
SHOULD_EQUAL("GetToken - multitoken - comment - EOF", token->type, STT_EOF);

overwriteFile(filePath, "$var /* comment \n comment2 */ $var2");
token = nextToken();
SHOULD_EQUAL("GetToken - multitoken - multiline comment - var1", token->type, STT_Variable);
token = nextToken();
SHOULD_EQUAL("GetToken - multitoken - multiline comment - var2", token->type, STT_Variable);
token = nextToken();
SHOULD_EQUAL("GetToken - multitoken - multiline comment - EOF", token->type, STT_EOF);

overwriteFile(filePath, "==");
token = nextToken();
SHOULD_EQUAL("GetToken - lex error double equal", token, NULL);
clearError();

overwriteFile(filePath, "if else elseif continue break while for true false return function null");
token = nextToken();
SHOULD_EQUAL("GetToken - keywordType - If", token->keywordType, KTT_If);
token = nextToken();
SHOULD_EQUAL("GetToken - keywordType - Else", token->keywordType, KTT_Else);
token = nextToken();
SHOULD_EQUAL("GetToken - keywordType - ElseIf", token->keywordType, KTT_Elseif);
token = nextToken();
SHOULD_EQUAL("GetToken - keywordType - Continue", token->keywordType, KTT_Continue);
token = nextToken();
SHOULD_EQUAL("GetToken - keywordType - Break", token->keywordType, KTT_Break);
token = nextToken();
SHOULD_EQUAL("GetToken - keywordType - While", token->keywordType, KTT_While);
token = nextToken();
SHOULD_EQUAL("GetToken - keywordType - For", token->keywordType, KTT_For);
token = nextToken();
SHOULD_EQUAL("GetToken - type - True", token->type, STT_Bool);
SHOULD_EQUAL("GetToken - value - True", token->n, 1);
token = nextToken();
SHOULD_EQUAL("GetToken - type - False", token->type, STT_Bool);
SHOULD_EQUAL("GetToken - value - False", token->n, 0);
token = nextToken();
SHOULD_EQUAL("GetToken - keywordType - Return", token->keywordType, KTT_Return);
token = nextToken();
SHOULD_EQUAL("GetToken - keywordType - Function", token->keywordType, KTT_Function);
token = nextToken();
SHOULD_EQUAL("GetToken - keywordType - Null", token->type, STT_Null);

// @todo need tests for symbol table.

if (token)
    freeToken(&token);
scannerReset();

TEST_SUITE_END
