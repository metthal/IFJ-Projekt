#ifndef BUILTIN_H
#define BUILTIN_H

#include "symbol.h"

uint8_t valueToBool(const Value *val);
int valueToInt(const Value *val);

// Ret must be initialized
void boolval(const Value *val, Value *ret);
void doubleval(const Value *val, Value *ret);
void findString(Value *ret, const Value *a, const Value *b);
void getString(Value *ret);
void getSubstring(const Value *val, Value *ret, int start, int end);
void intval(const Value *val, Value *ret);
void putString(Value *ret, Value *firstVal, int count);
void sortString(const Value *val, Value *ret);
void strLen(const Value *val, Value *ret);
void strval(const Value *val, Value *ret);

#endif
