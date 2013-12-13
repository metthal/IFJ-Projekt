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
 * @file value_vector.h
 * @brief Value vector specialization.
 **/

#ifndef VALUE_VECTOR_H
#define VALUE_VECTOR_H

#include "vector.h"
#include "symbol.h"

typedef Value* ValueVectorIterator;
typedef const Value* ConstValueVectorIterator;

#define VECTOR_STRUCT_ITEM
#define VECTOR_ITEM Value
#define VECTOR_ITERATOR ValueVectorIterator
#include "vector_template.h"
#undef VECTOR_ITERATOR
#undef VECTOR_ITEM
#undef VECTOR_STRUCT_ITEM

/**
 * Optimized vector operation to push item from the same vector.
 * @param vec Vector to operate on.
 * @param index Index of value to push on top.
 * @return Iterator to pushed value.
 */
static inline ValueVectorIterator vectorPushIndexValue(Vector *vec, uint32_t index)
{
    if (index >= vec->size) {
        setError(ERR_Range);
        return NULL;
    }

    if (vec->size == vec->capacity) {
        if (vec->capacity == 0)
            vectorReserve(vec, VectorDefaultCapacity);
        else
            vectorReserve(vec, vec->capacity * VectorResizeIncRate);
        if (getError())
            return NULL;
    }

    copyValue((const Value*)(vec->data + index * vec->itemSize), (Value*)vec->end);
    Value *ret = (Value*)vec->end;

    if (ret->type == VT_StrongReference || ret->type == VT_WeakReference) {
        // Reference needs to be updated
        ret->data.ref += ((int64_t)index - vec->size);
    }

    vec->size++;
    vec->end += vec->itemSize;
    return ret;
}

/**
 * Optimized vector operation to push N values at the same time.
 * @param vec Vector to operate on.
 * @param count Number of default values to be pushed - N.
 */
static inline void vectorPushDefaultNValue(Vector *vec, uint32_t count)
{
    if (vec->size + count > vec->capacity) {
        uint32_t newCapacity = vec->capacity;

        if (vec->capacity == 0)
            newCapacity = VectorDefaultCapacity;

        while (vec->size + count > newCapacity)
            newCapacity *= VectorResizeIncRate;

        vectorReserve(vec, newCapacity);
        if (getError())
            return;
    }

    // Initialize values
    memset(vec->end, 0, count * sizeof(Value));

    vec->size += count;
    vec->end += count * sizeof(Value);
}

/**
 * Optimized vector operation to specifically downsize vector.
 * @param vec Vector to operate on
 * @param size New vector size after downsize.
 */
static inline void vectorDownsizeValue(Vector *vec, uint32_t size)
{
    if (size == vec->size)
        return;

    Value *newEnd = ((Value *)vec->data) + size;
    for (Value *it = newEnd; it != ((Value *)vec->end); it++)
        deleteValue(it);

    vec->size = size;
    vec->end = (uint8_t*)newEnd;
}

/**
 * Optimized operation to retrieve value without overhead.
 * @param vec Vector to operate on.
 * @param index Index of value to return.
 * @return Iterator onto querried value.
 */
static inline ValueVectorIterator vectorFastAtValue(Vector *vec, uint32_t index)
{
    return ((Value*)vec->data) + index;
}

#endif
