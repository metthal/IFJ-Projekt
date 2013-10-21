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
    resetScanner();
    openFile(filePath);
    fclose(file);
    return 1;
}

TEST_SUITE_START(ScannerTests);

generateFilePath();

SHOULD_EQUAL("openFile - not existing file", openFile(filePath), NULL);

overwriteFile(filePath, "");
SHOULD_NOT_EQUAL("openFile - existing file", openFile(filePath), NULL);

token = scannerGetToken();
SHOULD_EQUAL("GetToken - empty file - EOF", token->type, STT_EOF);

overwriteFile(filePath, ";");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - semicolon", token->type, STT_Semicolon);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - semicolon - EOF", token->type, STT_EOF);

overwriteFile(filePath, "{");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - left curly bracket", token->type, STT_LeftCurlyBracket);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - left curly bracket - EOF", token->type, STT_EOF);

overwriteFile(filePath, "*");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - asterisk", token->type, STT_Asterisk);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - asterisk - EOF", token->type, STT_EOF);

overwriteFile(filePath, "}");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - right curly bracket", token->type, STT_RightCurlyBracket);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - right curly bracket - EOF", token->type, STT_EOF);

overwriteFile(filePath, "+");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - plus", token->type, STT_Plus);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - plus - EOF", token->type, STT_EOF);

overwriteFile(filePath, "-");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - minus", token->type, STT_Minus);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - minus - EOF", token->type, STT_EOF);

overwriteFile(filePath, ")");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - right bracket", token->type, STT_RightBracket);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - right bracket - EOF", token->type, STT_EOF);

overwriteFile(filePath, "(");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - left bracket", token->type, STT_LeftBracket);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - left bracket - EOF", token->type, STT_EOF);

overwriteFile(filePath, "id");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - identifier", token->type, STT_Identifier);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - identifier - EOF", token->type, STT_EOF);

overwriteFile(filePath, "$var");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - $var", token->type, STT_Variable);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - $var - EOF", token->type, STT_EOF);

overwriteFile(filePath, "1337");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - number", token->type, STT_Number);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - number - EOF", token->type, STT_EOF);

overwriteFile(filePath, "function");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - keyword (function)", token->type, STT_Keyword);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - keyword (function) - EOF", token->type, STT_EOF);

overwriteFile(filePath, ",");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - comma", token->type, STT_Comma);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - comma - EOF", token->type, STT_EOF);

overwriteFile(filePath, "=");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - assignment", token->type, STT_Assignment);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - assignment - EOF", token->type, STT_EOF);

overwriteFile(filePath, ">");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - greater", token->type, STT_Greater);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - greater - EOF", token->type, STT_EOF);

overwriteFile(filePath, ">=");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - greater equal", token->type, STT_GreaterEqual);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - greater equal - EOF", token->type, STT_EOF);

overwriteFile(filePath, "<");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - less", token->type, STT_Less);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - less - EOF", token->type, STT_EOF);

overwriteFile(filePath, "<=");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - less equal", token->type, STT_LessEqual);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - less equal - EOF", token->type, STT_EOF);

overwriteFile(filePath, "===");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - equal", token->type, STT_Equal);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - equal - EOF", token->type, STT_EOF);

overwriteFile(filePath, "!==");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - not equal", token->type, STT_NotEqual);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - not equal - EOF", token->type, STT_EOF);

overwriteFile(filePath, "\"string\"");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - string", token->type, STT_String);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - string - EOF", token->type, STT_EOF);

overwriteFile(filePath, "12.03");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - double", token->type, STT_Double);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - double - EOF", token->type, STT_EOF);

overwriteFile(filePath, "/");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - divide", token->type, STT_Divide);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - divide - EOF", token->type, STT_EOF);

overwriteFile(filePath, "<?php");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - <?php", token->type, STT_Php);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - <?php - EOF", token->type, STT_EOF);

overwriteFile(filePath, "var 123;");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - multitoken - id", token->type, STT_Identifier);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - multitoken - number", token->type, STT_Number);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - multitoken - semicolon", token->type, STT_Semicolon);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - multitoken - EOF", token->type, STT_EOF);

overwriteFile(filePath, "12.");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - lex error 1", token, NULL);

TEST_SUITE_END
