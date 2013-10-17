#include "nierr.h"

/** Global interpret error variable. */
NiError niErr;

void printError()
{
    if(niErr.type == ERR_None) return;

    fprintf(stderr, "Error at %s:%d:%s: ", niErr.file, niErr.line, niErr.fun);
    switch(niErr.type)
    {
    case ERR_Unknown:
        fprintf(stderr, "Unknown error.\n");
        break;
    default:
        fprintf(stderr, "Undocumented error.\n");
        break;
    }
}
