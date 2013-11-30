#ifndef BUILTIN_H
#define BUILTIN_H

#include "string.h"
#include "symbol.h"

typedef enum
{
    BTI_None = 0,
    BTI_BoolVal,
    BTI_DoubleVal,
    BTI_FindString,
    BTI_GetString,
    BTI_GetSubstring,
    BTI_IntVal,
    BTI_PutString,
    BTI_SortString,
    BTI_StrLen,
    BTI_StrVal,
} BuiltinCode;

BuiltinCode getBuiltinCode(const String *str);
int64_t getBuiltinParamCount(BuiltinCode code);

uint8_t valueToBool(const Value *val);
int valueToInt(const Value *val);
void valueToString(const Value *val, String *str);

// Ret must be initialized
void boolval(const Value *val, Value *ret);
void doubleval(const Value *val, Value *ret);
void findString(Value *ret, const Value *a, const Value *b);
void getString(Value *ret);
void getSubstring(const Value *val, Value *ret, int start, int end);
void intval(const Value *val, Value *ret);
void putString(const Value *base, const Value *constBase, Value *ret, int count);
void sortString(const Value *val, Value *ret);
void strLen(const Value *val, Value *ret);
void strval(const Value *val, Value *ret);

#endif
