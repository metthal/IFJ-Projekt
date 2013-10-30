#include "nierr.h"
#include "rc.h"
#include "scanner.h"
#include "parser.h"

uint8_t processParams(int32_t argc, char **argv, Vector** tokenVector)
{
    if (argc != 2){
        *tokenVector = NULL;
        return 1;
    }

    *tokenVector = scannerScanFile(argv[1]);
    return 0;
}

int main(int argc, char **argv)
{
    setError(ERR_None);

    Vector* tokenVector = NULL;
    if (processParams(argc, argv, &tokenVector))
        return RC_FatalError;

    if(getError())
        return getRcFromError();

    parse(tokenVector);

    return getRcFromError();
}
