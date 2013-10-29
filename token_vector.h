#ifndef TOKEN_VECTOR_H
#define TOKEN_VECTOR_H

#include "vector.h"

#define VECTOR_STRUCT_ITEM
#define VECTOR_ITEM_HEADER "token.h"
#define VECTOR_ITEM Token
#include "vector_template.h"
#undef VECTOR_ITEM
#undef VECTOR_ITEM_HEADER
#undef VECTOR_STRUCT_ITEM

#endif
