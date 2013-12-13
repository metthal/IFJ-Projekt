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
 * @file uint32_vector.h
 * @brief Unsigned integer uint32_t vector specialization.
 **/

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
