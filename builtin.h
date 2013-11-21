#ifndef BUILTIN_H
#define BUILTIN_H

#include "symbol.h"

// Ret must be initialized or "after-deletion"
void boolval(const Value *val, Value *ret);
void doubleval(const Value *val, Value *ret);
void findString();
void getString();
void getSubstring();
void intval(const Value *val, Value *ret);
void putString();
void sortString();
void strLen();
void strval(const Value *val, Value *ret);

#endif
