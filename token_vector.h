#ifndef TOKEN_VECTOR_H
#define TOKEN_VECTOR_H

#include "vector.h"

#define STRUCT_ITEM
#define ITEM_HEADER "token.h"
#define ITEM Token
#define SMALL_ITEM token
#include "vector_template.h"
#undef SMALL_ITEM
#undef ITEM
#undef HEADER
#undef STRUCT_ITEM

#endif
