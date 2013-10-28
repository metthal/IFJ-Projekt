#ifndef VECTOR_TEMPLATE_H
#define VECTOR_TEMPLATE_H

#ifdef ITEM_HEADER
#include ITEM_HEADER
#endif

#include "nierr.h"

#include <stdint.h>
#include <stdlib.h>

static uint8_t const VectorResizeIncRate = 2;
static uint8_t const VectorResizeDecRate = 3;
static uint16_t const VectorDefaultCapacity = 16;

#define MAKE_NAME(x, y, z) x ##y ##z
#define EXPAND(x, y, z) MAKE_NAME(x, y, z)
#define TEMPLATE(x) EXPAND(x, ITEM, )
#define CONSTRUCT(x, y) EXPAND(x, ITEM, y)

#ifdef STRUCT_ITEM
#define PUSH_BACK_ITEM ITEM*
#define COPY_ITEM(src, dest) EXPAND(SMALL_ITEM, Copy, )((ITEM*)(src), (ITEM*)(dest))
#define INIT_ITEM(x) EXPAND(init, ITEM, )((ITEM*)(x))
#define DELETE_ITEM(x) EXPAND(delete, ITEM, )((ITEM*)(x))
#else
#define PUSH_BACK_ITEM ITEM
#define COPY_ITEM(src, dest) *(dest) = src
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
        setError(ERR_NewFailed);
        return NULL;
    }

    vec->size = 0;
    vec->capacity = VectorDefaultCapacity;
    vec->itemSize = sizeof(ITEM);
    vec->data = malloc(VectorDefaultCapacity * vec->itemSize);
    memset(vec->data, 0, VectorDefaultCapacity * vec->itemSize);

    if (vec->data == NULL) {
        setError(ERR_NewFailed);
        free(vec);
        return NULL;
    }

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
static void TEMPLATE(vectorPush)(Vector *vec, PUSH_BACK_ITEM item)
{
    if (vec->size == vec->capacity) {
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
static void TEMPLATE(vectorPushDefault)(Vector *vec)
{
    if (vec->size == vec->capacity) {
        vectorReserve(vec, vec->capacity * VectorResizeIncRate);
        if (getError())
            return;
    }

    INIT_ITEM(vec->end);
    vec->size++;
    vec->end += vec->itemSize;
}

/**
 * Removes last item from vector.
 * @param vec Vector to operate on.
 */
static void TEMPLATE(vectorPop)(Vector *vec)
{
    vec->size--;
    vec->end -= vec->itemSize;
    DELETE_ITEM(vec->end);
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
    uint8_t* newEnd = NULL;
    if (size == vec->size)
        return;
    else if (size > vec->size) {
        uint32_t newCapacity = vec->capacity;
        while (size > newCapacity)
            newCapacity *= VectorResizeIncRate;

        if (newCapacity != vec->capacity) {
            vectorReserve(vec, newCapacity);
            if (getError())
                return;
        }

        newEnd = vec->data + size * vec->itemSize;
        for (uint8_t* it = vec->end; it != newEnd; it += vec->itemSize)
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

#undef ITEM_COPY
#undef PUSH_BACK_ITEM
#undef CONSTRUCT
#undef TEMPLATE
#undef EXPAND
#undef MAKE_NAME

#endif
