#ifndef VALUE_VECTOR_H
#define VALUE_VECTOR_H

#include "vector.h"
#include "symbol.h"

typedef Value* ValueVectorIterator;
typedef const Value* ConstValueVectorIterator;

#define VECTOR_STRUCT_ITEM
#define VECTOR_ITEM Value
#define VECTOR_ITERATOR ValueVectorIterator
#include "vector_template.h"
#undef VECTOR_ITERATOR
#undef VECTOR_ITEM
#undef VECTOR_STRUCT_ITEM

#endif
