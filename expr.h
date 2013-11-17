#ifndef EXPR_H
#define EXPR_H

#include "parser.h"
#include "vector.h"
#include "token.h"

typedef enum
{
    Terminal,
    NonTerminal,
} ExprTokenType;

typedef struct
{
    Token *token;
    uint8_t type;
} ExprToken;

void initExpr();
void deinitExpr();
int32_t expr();

void initExprToken(ExprToken *token);
void deleteExprToken(ExprToken *token);
void copyExprToken(ExprToken *src, ExprToken *dest);

typedef ExprToken* ExprTokenVectorIterator;
typedef const ExprToken* ConstExprTokenVectorIterator;

#define VECTOR_STRUCT_ITEM
#define VECTOR_ITEM ExprToken
#define VECTOR_ITERATOR ExprTokenVectorIterator
#include "vector_template.h"
#undef VECTOR_ITERATOR
#undef VECTOR_ITEM
#undef VECTOR_STRUCT_ITEM

#endif
