#ifndef UINT32_T_VECTOR_H
#define UINT32_T_VECTOR_H

#include "vector.h"

typedef uint32_t Uint32;
typedef Uint32* Uint32Iterator;

#define VECTOR_ITEM Uint32
#define VECTOR_ITERATOR Uint32Iterator
#include "vector_template.h"
#undef VECTOR_ITERATOR
#undef VECTOR_ITEM

#endif
