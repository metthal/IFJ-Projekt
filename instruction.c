#include "instruction.h"
#include <stdlib.h>
#include <string.h>

Instruction* newInstruction()
{
    Instruction *tmp = malloc(sizeof(Instruction));
    memset(tmp, 0, sizeof(Instruction));
    return tmp;
}

void initInstruction(Instruction *pt)
{
    memset(pt, 0, sizeof(Instruction));
}

void deleteInstruction(Instruction *pt)
{
    if (pt != NULL) {

    }
}

void freeInstruction(Instruction **ppt)
{
    if (ppt != NULL) {
        if (*ppt != NULL) {

        }
        free(*ppt);
        *ppt = NULL;
    }
}

void copyInstruction(Instruction *src, Instruction *dest)
{
    // TODO todo...
    dest = src;
}
