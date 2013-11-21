#include "vector.h"
#include "nierr.h"
#include <stdlib.h>

void vectorReserve(Vector *vec, uint32_t capacity)
{
    // Would require deallocation of lost members
    if (capacity <= vec->capacity)
        return;

    void *tmp = realloc(vec->data, vec->itemSize * capacity);

    if (tmp == NULL) {
        setError(ERR_Allocation);
        return;
    }

    vec->data = tmp;
    vec->end = vec->data + vec->size * vec->itemSize;
    vec->capacity = capacity;
}

void vectorShrinkToFit(Vector *vec)
{
    void *tmp = realloc(vec->data, vec->itemSize * vec->size);
    if (tmp == NULL) {
        setError(ERR_Allocation);
        return;
    }

    vec->data = tmp;
    vec->end = vec->data + vec->size * vec->itemSize;
    vec->capacity = vec->size;
}

uint32_t vectorCapacity(Vector *vec)
{
    return vec->capacity;
}

uint32_t vectorSize(Vector *vec)
{
    return vec->size;
}

void* vectorAt(Vector *vec, uint32_t index)
{
    if (vec->size <= index) {
        setError(ERR_Range);
        return NULL;
    }
    return vec->data + vec->itemSize * index;
}

const void* vectorAtConst(const Vector *vec, uint32_t index)
{
    if (vec->size <= index) {
        setError(ERR_Range);
        return NULL;
    }
    return vec->data + vec->itemSize * index;
}

void* vectorFront(Vector *vec)
{
    if (vec->size == 0) {
        setError(ERR_Range);
        return NULL;
    }
    return vec->data;
}

void* vectorBack(Vector *vec)
{
    if (vec->size == 0) {
        setError(ERR_Range);
        return NULL;
    }
    return vec->end - vec->itemSize;
}
