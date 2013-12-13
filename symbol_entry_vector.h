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
 * @file symbol_entry_vector.h
 * @brief Symbol entry vector specialization.
 **/

#ifndef SYMBOLENTRY_VECTOR_H
#define SYMBOLENTRY_VECTOR_H

#include "vector.h"
#include "ial.h"

typedef SymbolEntry* SymbolEntryVectorIterator;
typedef const SymbolEntry* ConstSymbolEntryVectorIterator;

#define VECTOR_STRUCT_ITEM
#define VECTOR_ITEM SymbolEntry
#define VECTOR_ITERATOR SymbolEntryVectorIterator
#include "vector_template.h"
#undef VECTOR_ITERATOR
#undef VECTOR_ITEM
#undef VECTOR_STRUCT_ITEM

#endif
