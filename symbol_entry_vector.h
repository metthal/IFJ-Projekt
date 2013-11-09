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
