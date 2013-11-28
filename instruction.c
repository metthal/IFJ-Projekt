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

InstructionCode builtinToInstruction(BuiltinCode builtinCode)
{
    switch (builtinCode) {
        case BTI_None:
            return IST_Call;
        case BTI_BoolVal:
            return IST_BoolVal;
        case BTI_DoubleVal:
            return IST_DoubleVal;
        case BTI_FindString:
            return IST_FindString;
        case BTI_GetString:
            return IST_GetString;
        case BTI_GetSubstring:
            return IST_GetSubstring;
        case BTI_IntVal:
            return IST_IntVal;
        case BTI_SortString:
            return IST_SortString;
        case BTI_StrLen:
            return IST_StrLen;
        case BTI_StrVal:
            return IST_StrVal;
        case BTI_PutString:
            return IST_PutString;
    }
    return IST_Noop;
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

void generateCall(const Symbol *symbol, BuiltinCode builtinCode, uint32_t paramCount)
{
    Instruction inst;
    initInstruction(&inst);
    uint8_t undef = 1, badParamCount = 0;

    switch(builtinCode) {
        case BTI_None:
            // Too many parameters are clipped

            // Default parameters
            processDefaultArg(&(symbol->data->func.context), &paramCount);

            // Test for too few arguments
            if (symbol->data->func.context.argumentCount == paramCount)
                inst.a = symbol->data->func.functionAddressIndex;
            else
                badParamCount = 1;

            undef = 0;
            break;

        case BTI_PutString:
            // Set number of parameters as operand.
            inst.a = paramCount;
            undef = 0;
            break;

        default:
            // TODO will be paramCount equal or greater when too much?
            if (paramCount != getBuiltinParamCount(builtinCode))
                badParamCount = 1;

            undef = 0;
            break;
    }

    if (undef)
        setError(ERR_UndefFunction);
    else if (badParamCount)
        setError(ERR_BadParamCount);
    else {
        inst.code = builtinToInstruction(builtinCode);
        vectorPushInstruction(instructions, &inst);
    }
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

Symbol* fillInstFuncInfo(const Token *funcToken, BuiltinCode *builtinCode, int64_t *paramCount)
{
    Symbol *symbol = NULL;
    *builtinCode = getBuiltinCode(&(funcToken->str));

    if (*builtinCode == BTI_None) {
        symbol = symbolTableFind(globalSymbolTable, &funcToken->str);
        if (symbol == NULL) {
            setError(ERR_UndefFunction);
            return NULL;
        }

        *paramCount = symbol->data->func.context.argumentCount;
    }
    else
        *paramCount = getBuiltinParamCount(*builtinCode);

    return symbol;
}
