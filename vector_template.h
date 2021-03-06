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
 * @file vector_template.h
 * @brief Template file for creating specialized vectors.
 **/

#ifdef VECTOR_ITEM_HEADER
#include VECTOR_ITEM_HEADER
#endif

#ifndef VECTOR_TEMPLATE_H
#define VECTOR_TEMPLATE_H
#include "nierr.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/** Factor used when enlarging vector. */
static uint8_t const VectorResizeIncRate = 2;
/** Factor used when shrinking vector. */
static uint8_t const VectorResizeDecRate = 3;
/** Default vector capacity upon creation. */
static uint16_t const VectorDefaultCapacity = 16;
#endif

#define MAKE_NAME(x, y, z) x ##y ##z
#define EXPAND(x, y, z) MAKE_NAME(x, y, z)
#define TEMPLATE(x) EXPAND(x, VECTOR_ITEM, )
#define CONSTRUCT(x, y) EXPAND(x, VECTOR_ITEM, y)

#ifdef VECTOR_STRUCT_ITEM
#define PUSH_BACK_ITEM VECTOR_ITEM*
#define COPY_ITEM(src, dest) TEMPLATE(copy)((src), (VECTOR_ITERATOR)(dest))
#define INIT_ITEM(x) TEMPLATE(init)((VECTOR_ITEM*)(x))
#define DELETE_ITEM(x) TEMPLATE(delete)((VECTOR_ITEM*)(x))
#else
#define PUSH_BACK_ITEM VECTOR_ITEM
#define COPY_ITEM(src, dest) *((VECTOR_ITERATOR)dest) = src
#define INIT_ITEM(x) (x) = 0
#define DELETE_ITEM(x)
#endif

/**
 * Constructs vector with default capacity.
 * @return Constructed vector.
 */
static Vector* CONSTRUCT(new, Vector)()
{
    Vector* vec = malloc(sizeof(Vector));

    if (vec == NULL) {
        setError(ERR_Allocation);
        return NULL;
    }

    vec->size = 0;
    vec->capacity = VectorDefaultCapacity;
    vec->itemSize = sizeof(VECTOR_ITEM);
    vec->data = malloc(VectorDefaultCapacity * vec->itemSize);

    if (vec->data == NULL) {
        setError(ERR_Allocation);
        free(vec);
        return NULL;
    }

    memset(vec->data, 0, vec->capacity * vec->itemSize);
    vec->end = vec->data;
    return vec;
}

/**
 * Frees content of all elements in vector and
 * destroys vector itself.
 * Passed pointer to vector is set to NULL.
 * @param vec Vector to free.
 */
static void CONSTRUCT(free, Vector)(Vector **vec)
{
    Vector* tmp = (*vec);
    for (uint8_t* it = tmp->data; it != tmp->end; it += tmp->itemSize)
        DELETE_ITEM(it);
    free(tmp->data);
    free(*vec);
    *vec = NULL;
}

/**
 * Frees content of all elements in vector.
 * Vector will not reallocate.
 * @param vec Vector to clear.
 */
static void TEMPLATE(vectorClear)(Vector *vec)
{
    for (uint8_t* it = vec->data; it != vec->end; it += vec->itemSize)
        DELETE_ITEM(it);
    vec->size = 0;
    vec->end = vec->data;
}

/**
 * Appends new item to the end of the vector.
 * If item is structure, it is copied using it's
 * copy method.
 * Sets error flag on error.
 * @param vec Vector to operate on.
 * @param item Item to append.
 */
static void TEMPLATE(vectorPush)(Vector *vec, const PUSH_BACK_ITEM item)
{
    if (vec->size == vec->capacity) {
        if (vec->capacity == 0)
            vectorReserve(vec, VectorDefaultCapacity);
        else
            vectorReserve(vec, vec->capacity * VectorResizeIncRate);
        if (getError())
            return;
    }

    COPY_ITEM(item, vec->end);
    vec->size++;
    vec->end += vec->itemSize;
}

/**
 * Appends new item to the end of the vector.
 * If item is structure, it's content is
 * initialized using it's default initializer.
 * Sets error flag on error.
 * @param vec Vector to operate on.
 */
static VECTOR_ITERATOR TEMPLATE(vectorPushDefault)(Vector *vec)
{
    if (vec->size == vec->capacity) {
        if (vec->capacity == 0)
            vectorReserve(vec, VectorDefaultCapacity);
        else
            vectorReserve(vec, vec->capacity * VectorResizeIncRate);
        if (getError())
            return NULL;
    }

    INIT_ITEM(vec->end);
    VECTOR_ITERATOR retItem = (VECTOR_ITERATOR)vec->end;
    vec->size++;
    vec->end += vec->itemSize;
    return retItem;
}

/**
 * Removes last item from vector.
 * @param vec Vector to operate on.
 */
static void TEMPLATE(vectorPop)(Vector *vec)
{
    if (vec->size > 0) {
        vec->size--;
        vec->end -= vec->itemSize;
        DELETE_ITEM(vec->end);
    }
}

/**
 * Removes N last items from vector.
 * @param vec Vector to operate on.
 * @param amount Number of items to pop
 */
static void TEMPLATE(vectorPopN)(Vector *vec, uint32_t amount)
{
    if (amount > vec->size)
        amount = vec->size;

    VECTOR_ITERATOR itr = (VECTOR_ITERATOR)(vec->end - vec->itemSize);
    for (uint32_t i = 1; i <= amount; ++i, --itr)
        DELETE_ITEM(itr);

    vec->size -= amount;
    vec->end -= amount * vec->itemSize;
}

/**
 * Resizes vector to specified size.
 * Sets ERR_Allocation on error and preserves
 * previous capacity and data content.
 * @param vec Vector to resize.
 * @param size New size of vector.
 */
static void TEMPLATE(vectorResize)(Vector *vec, uint32_t size)
{
    uint8_t *newEnd = NULL;
    if (size == vec->size)
        return;
    else if (size > vec->size) {
        uint32_t newCapacity = vec->capacity;

        if (vec->capacity == 0)
            newCapacity = VectorDefaultCapacity;

        while (size > newCapacity)
            newCapacity *= VectorResizeIncRate;

        if (newCapacity != vec->capacity) {
            vectorReserve(vec, newCapacity);
            if (getError())
                return;
        }

        newEnd = vec->data + size * vec->itemSize;
        for (uint8_t *it = vec->end; it != newEnd; it += vec->itemSize)
            INIT_ITEM(it);
    }
    else {
        // TODO: Shrink if capacity too big
        newEnd = vec->data + size * vec->itemSize;
        for (uint8_t* it = newEnd; it != vec->end; it += vec->itemSize)
            DELETE_ITEM(it);
    }

    vec->size = size;
    vec->end = newEnd;
}

/**
 * Returns iterator at the beginning of vector.
 * @param vec Vector to operate on.
 * @return Iterator at the beginning of vector.
 */
static VECTOR_ITERATOR TEMPLATE(vectorBegin)(Vector *vec)
{
    return (VECTOR_ITERATOR)vec->data;
}

/**
 * Returns iterator at the past-the-end element in
 * vector.
 * This means first element that is out of vector
 * range.
 * @param vec Vector to operate on.
 * @return Iterator at the end of vector.
 */
static VECTOR_ITERATOR TEMPLATE(vectorEnd)(Vector *vec)
{
    return (VECTOR_ITERATOR)vec->end;
}

#undef INIT_ITEM
#undef DELETE_ITEM
#undef COPY_ITEM
#undef PUSH_BACK_ITEM
#undef CONSTRUCT
#undef TEMPLATE
#undef EXPAND
#undef MAKE_NAME
