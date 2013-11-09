#ifndef INSTRUCTION_VECTOR_H
#define INSTRUCTION_VECTOR_H

#include "vector.h"
#include "instruction.h"

typedef Instruction* InstructionVectorIterator;
typedef const Instruction* ConstInstructionVectorIterator;

#define VECTOR_STRUCT_ITEM
#define VECTOR_ITEM Instruction
#define VECTOR_ITERATOR InstructionVectorIterator
#include "vector_template.h"
#undef VECTOR_ITERATOR
#undef VECTOR_ITEM
#undef VECTOR_STRUCT_ITEM

#endif
