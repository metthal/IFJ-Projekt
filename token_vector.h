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
 * @file token_vector.h
 * @brief Token vector specialization.
 **/

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
