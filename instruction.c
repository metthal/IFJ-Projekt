#include "instruction.h"
#include "symbol.h"
#include "ial.h"
#include "instruction_vector.h"
#include <stdlib.h>
#include <string.h>

extern SymbolTable *globalSymbolTable;
extern Vector *instructions;

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
        int64_t index = context->defaultStart + missingCount - 1;
        for (; index >= context->defaultStart; index--)
            generateInstruction(IST_PushC, 0, index, 0);

        *paramCount += missingCount;
    }
}

void generateCall(Symbol *symbol, InstructionCode instCode, uint32_t paramCount)
{
    Instruction inst;
    initInstruction(&inst);
    inst.code = instCode;
    uint8_t undef = 1, badParamCount = 0;

    switch(instCode) {
        case IST_BoolVal:
            if (paramCount != 1)
                badParamCount = 1;

            undef = 0;
            break;
        case IST_DoubleVal:
            if (paramCount != 1)
                badParamCount = 1;

            undef = 0;
            break;
        case IST_FindString:
            if (paramCount != 2)
                badParamCount = 1;

            undef = 0;
            break;
        case IST_GetString:
            if (paramCount != 0)
                badParamCount = 1;

            undef = 0;
            break;
        case IST_GetSubstring:
            if (paramCount != 3)
                badParamCount = 1;

            undef = 0;
            break;
        case IST_IntVal:
            if (paramCount != 1)
                badParamCount = 1;

            undef = 0;
            break;
        case IST_PutString:
            // Set number of parameters as operand.
            inst.a = paramCount;
            undef = 0;
            break;
        case IST_SortString:
            if (paramCount != 1)
                badParamCount = 1;

            undef = 0;
            break;
        case IST_StrLen:
            if (paramCount != 1)
                inst.code = IST_StrLen;

            undef = 0;
            break;
        case IST_StrVal:
            if (paramCount != 1)
                badParamCount = 1;

            undef = 0;
            break;
        case IST_Call:
            // Test for too many arguments
            if (symbol->data->func.context.argumentCount >= paramCount) {
                // Default parameters
                processDefaultArg(&(symbol->data->func.context), &paramCount);

                // Test for too few arguments
                if (symbol->data->func.context.argumentCount == paramCount)
                    inst.a = symbol->data->func.functionAddressIndex;
                else
                    badParamCount = 1;
            }
            else
                badParamCount = 1;

            undef = 0;
            break;
        default:
            return;
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

struct sSymbol* fillInstFuncInfo(Token *funcToken, InstructionCode *instCode, int64_t *paramCount)
{
    struct sSymbol *symbol = symbolTableFind(globalSymbolTable, &funcToken->str);
    if (symbol== NULL) {
        switch(funcToken->str.data[0]) {
            case 'b':
                if (stringCompareS(&funcToken->str, "boolval", 7) == 0) {
                    *instCode = IST_BoolVal;
                    *paramCount = 1;
                }
                break;
            case 'd':
                if (stringCompareS(&funcToken->str, "doubleval", 9) == 0) {
                    *instCode = IST_DoubleVal;
                    *paramCount = 1;
                }
                break;
            case 'f':
                if (stringCompareS(&funcToken->str, "find_string", 11) == 0) {
                    *instCode = IST_FindString;
                    *paramCount = 2;
                }
                break;
            case 'g':
                if (stringCompareS(&funcToken->str, "get_string", 10) == 0) {
                    *instCode = IST_GetString;
                    *paramCount = 0;
                }
                else if (stringCompareS(&funcToken->str, "get_substring", 13) == 0) {
                    *instCode = IST_GetSubstring;
                    *paramCount = 3;
                }
                break;
            case 'i':
                if (stringCompareS(&funcToken->str, "intval", 6) == 0) {
                    *instCode = IST_IntVal;
                    *paramCount = 1;
                }
                break;
            case 'p':
                if (stringCompareS(&funcToken->str, "put_string", 10) == 0) {
                    *instCode = IST_PutString;
                    *paramCount = -1;
                }
                break;
            case 's':
                if (stringCompareS(&funcToken->str, "sort_string", 11) == 0) {
                    *instCode = IST_SortString;
                    *paramCount = 1;
                }
                else if (stringCompareS(&funcToken->str, "strlen", 6) == 0) {
                    *instCode = IST_StrLen;
                    *paramCount = 1;
                }
                else if (stringCompareS(&funcToken->str, "strval", 6) == 0) {
                    *instCode = IST_StrVal;
                    *paramCount = 1;
                }
                break;
            default:
                *instCode = IST_Noop;
                *paramCount = 0;
                break;
        }
    }
    else {
        *instCode = IST_Call;
        *paramCount = symbol->data->func.context.argumentCount;
    }

    return symbol;
}
