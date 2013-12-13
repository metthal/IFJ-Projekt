/*
 * Project name:
 * Implementace interpretu imperativního jazyka IFJ13.
 *
 * Codename:
 * INI: Ni Interpreter
 *
 * Description:
 * https://wis.fit.vutbr.cz/FIT/st/course-files-st.php/course/IFJ-IT/projects/ifj2013.pdf
 *
 * Project's GitHub repository:
 * https://github.com/metthal/IFJ-Projekt
 *
 * Team:
 * Marek Milkovič   (xmilko01)
 * Lukáš Vrabec     (xvrabe07)
 * Ján Spišiak      (xspisi03)
 * Ivan Ševčík      (xsevci50)
 * Marek Bertovič   (xberto00)
 */

/**
 * @file expr.h
 * @brief Declares interface of the bottom-up parser
 **/
#ifndef EXPR_H
#define EXPR_H

#include "parser.h"
#include "vector.h"
#include "token.h"

/**
 * Defines type of the token located on the bottom-up parser stack
 **/
typedef enum
{
    Terminal,   ///< Terminal token
    NonTerminal ///< Non-terminal token
} ExprTokenType;

typedef enum
{
    NonConst,
    Const
} ExprTokenSubtype;

/**
 * Expression token used in bottom-up parsing
 **/
typedef struct
{
    union
    {
        Token *token;
        int64_t offset;
    };
    ExprTokenType type;
    ExprTokenSubtype subtype;
} ExprToken;

void initExpr();
void deinitExpr();
int64_t expr(int64_t resultOffset, uint32_t *maxStackPosUsed);

void initExprToken(ExprToken *token);
void deleteExprToken(ExprToken *token);
void copyExprToken(const ExprToken *src, ExprToken *dest);

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
