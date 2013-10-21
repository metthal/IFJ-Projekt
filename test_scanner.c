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

overwriteFile(filePath, "<?php");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - <?php", token->type, STT_Php);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - <?php - EOF", token->type, STT_EOF);

overwriteFile(filePath, "$var");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - $var", token->type, STT_Variable);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - $var - EOF", token->type, STT_EOF);

overwriteFile(filePath, "12.03");
token = scannerGetToken();
SHOULD_EQUAL("GetToken - double", token->type, STT_Double);
token = scannerGetToken();
SHOULD_EQUAL("GetToken - double - EOF", token->type, STT_EOF);

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
