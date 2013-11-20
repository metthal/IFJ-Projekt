#include "instruction.h"
#include "symbol.h"
#include "ial.h"
#include "instruction_vector.h"
#include <stdlib.h>
#include <string.h>

extern SymbolTable *globalSymbolTable;
extern Vector *instructions;
extern Context *currentContext;

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

void copyInstruction(const Instruction *src, Instruction *dest)
{
    dest->code = src->code;
    dest->res = src->res;
    dest->a = src->a;
    dest->b = src->b;
}

void processDefaultArg(Context *context, uint32_t *paramCount)
{
    uint32_t missingCount = context->argumentCount - *paramCount;
    if (missingCount > 0 && missingCount <= context->defaultCount) {
        // First argument is first on stack, just append defaults
        // which are in constant table in opposite order (last is first)
        // TODO Use IST_PushC in cycle
        *paramCount += missingCount;
    }
}

// TODO bottom up parser still needs to reserve space for return value
void generateCall(const Token *id, uint32_t paramCount)
{
    Instruction inst;
    initInstruction(&inst);
    uint8_t undef = 1, badParamCount = 0;

    Symbol *symbol = symbolTableFind(globalSymbolTable, &id->str);
    if (symbol == NULL) {
        switch(id->str.data[0]) {
            case 'b':
                if (stringCompareS(&id->str, "boolval", 7) == 0) {
                    if (paramCount == 1)
                        inst.code = IST_BoolVal;
                    else
                        badParamCount = 1;
                    undef = 0;
                }
                break;
            case 'd':
                if (stringCompareS(&id->str, "doubleval", 9) == 0) {
                    if (paramCount == 1)
                        inst.code = IST_DoubleVal;
                    else
                        badParamCount = 1;
                    undef = 0;
                }
                break;
            case 'f':
                if (stringCompareS(&id->str, "find_string", 11) == 0) {
                    if (paramCount == 2)
                        inst.code = IST_FindString;
                    else
                        badParamCount = 1;
                    undef = 0;
                }
                break;
            case 'g':
                if (stringCompareS(&id->str, "get_string", 10) == 0) {
                    if (paramCount == 0)
                        inst.code = IST_GetString;
                    else
                        badParamCount = 1;
                    undef = 0;
                }
                else if (stringCompareS(&id->str, "get_substring", 13) == 0) {
                    if (paramCount == 3)
                        inst.code = IST_GetSubstring;
                    else
                        badParamCount = 1;
                    undef = 0;
                }
                break;
            case 'i':
                if (stringCompareS(&id->str, "intval", 6) == 0) {
                    if (paramCount == 1)
                        inst.code = IST_IntVal;
                    else
                        badParamCount = 1;
                    undef = 0;
                }
                break;
            case 'p':
                if (stringCompareS(&id->str, "put_string", 10) == 0) {
                    // Set number of parameters as operand.
                    inst.code = IST_PutString;
                    inst.a = paramCount;
                    undef = 0;
                }
                break;
            case 's':
                if (stringCompareS(&id->str, "sort_string", 11) == 0) {
                    if (paramCount == 1)
                        inst.code = IST_SortString;
                    else
                        badParamCount = 1;
                    undef = 0;
                }
                else if (stringCompareS(&id->str, "strlen", 6) == 0) {
                    if (paramCount == 1)
                        inst.code = IST_StrLen;
                    else
                        badParamCount = 1;
                    undef = 0;
                }
                else if (stringCompareS(&id->str, "strval", 6) == 0) {
                    if (paramCount == 1)
                        inst.code = IST_StrVal;
                    else
                        badParamCount = 1;
                    undef = 0;
                }
                break;
        }
    }
    else {
        // Test for too many arguments
        if (symbol->data->func.context.argumentCount >= paramCount) {
            // Default parameters
            processDefaultArg(&(symbol->data->func.context), &paramCount);

            // Test for too few arguments
            if (symbol->data->func.context.argumentCount == paramCount) {
                inst.code = IST_Call;
                inst.a = symbol->data->func.functionAddressIndex;
            }
            else
                badParamCount = 1;
        }
        else
            badParamCount = 1;

        undef = 0;
    }

    if (undef)
        setError(ERR_UndefFunction);
    else if (badParamCount)
        setError(ERR_BadParamCount);
    else
        vectorPushInstruction(instructions, &inst);
}

void generateInstruction(InstructionCode code, int32_t res, int32_t a, int32_t b)
{
    if (code == IST_Call) {
        setError(ERR_ISTGenerator);
        return;
    }

    Instruction inst;
    inst.code = code;
    inst.res = res;
    inst.a = a;
    inst.b = b;
    vectorPushInstruction(instructions, &inst);
}

uint32_t generateEmptyInstruction()
{
    uint32_t ret = vectorSize(instructions);
    vectorPushDefaultInstruction(instructions);
    return ret;
}

void fillInstruction(uint32_t index, InstructionCode code, int32_t res, int32_t a, int32_t b)
{
    if (code == IST_Call) {
        setError(ERR_ISTGenerator);
        return;
    }

    Instruction *pt = vectorAt(instructions, index);
    if (pt->code == IST_Noop || pt->code == IST_Break || pt->code == IST_Continue) {
        pt->code = code;
        pt->res = res;
        pt->a = a;
        pt->b = b;
    }
}
