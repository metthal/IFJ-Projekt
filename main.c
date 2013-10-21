#include "nierr.h"
#include "rc.h"
#include "scanner.h"

uint8_t processParams(int32_t argc, char **argv)
{
    if (argc != 2)
        return 0;

    return (scannerOpenFile(argv[1]) != NULL);
}

int main(int argc, char **argv)
{
    setError(ERR_None);

    if (!processParams(argc, argv))
        return RC_FatalError;

    ///< @todo Start parser

    return getRcFromError();
}
