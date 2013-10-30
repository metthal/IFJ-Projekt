#ifndef TOKEN_VECTOR_H
#define TOKEN_VECTOR_H

#include "vector.h"
#include "token.h"

typedef Token* TokenVectorIterator;
typedef const Token* ConstTokenVectorIterator;

#define VECTOR_STRUCT_ITEM
#define VECTOR_ITEM Token
#define VECTOR_ITERATOR TokenVectorIterator
#include "vector_template.h"
#undef VECTOR_ITERATOR
#undef VECTOR_ITEM
#undef VECTOR_STRUCT_ITEM

#endif
