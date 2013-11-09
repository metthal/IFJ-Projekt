#ifndef ADDRESS_VECTOR_H
#define ADDRESS_VECTOR_H

#include "vector.h"
#include "instruction.h"

typedef Instruction* InstructionPtr;
typedef InstructionPtr* InstructionPtrVectorIterator;
typedef const InstructionPtr* ConstInstructionPtrVectorIterator;

#define VECTOR_ITEM InstructionPtr
#define VECTOR_ITERATOR InstructionPtrVectorIterator
#include "vector_template.h"
#undef VECTOR_ITERATOR
#undef VECTOR_ITEM
#undef VECTOR_STRUCT_ITEM

#endif
