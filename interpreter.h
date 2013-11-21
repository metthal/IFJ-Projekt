#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "vector.h"
#include "instruction.h"

void interpret(const Instruction *firstInstruction, const Vector *constTable, const Vector *addressTable);

#endif
