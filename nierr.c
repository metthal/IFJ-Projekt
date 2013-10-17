#include "nierr.h"

/** Global interpret error variable. */
NiError niErr;

void printError()
{
	if(niErr.type == ERR_None) return;

	printf("Error at %s:%d:%s: ", niErr.file, niErr.line, niErr.fun);
	switch(niErr.type)
	{
	case ERR_Unknown:
		printf("Unknown error.\n");
		break;
	default:
		printf("Undocumented error.\n");
		break;
	}
}
