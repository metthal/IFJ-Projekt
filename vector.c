#include "vector.h"
#include "nierr.h"
#include <stdlib.h>
#include <string.h>

void vectorReserve(Vector *vec, uint32_t capacity)
{
    // Would require deallocation of lost members
    if (capacity <= vec->capacity)
        return;

    void *tmp;

    if (vec->capacity > 0)
        tmp = realloc(vec->data, vec->itemSize * capacity);
    else
        tmp = malloc(vec->itemSize * capacity);

    if (tmp == NULL) {
        setError(ERR_Allocation);
        return;
    }

    vec->data = tmp;
    vec->end = vec->data + vec->size * vec->itemSize;
    vec->capacity = capacity;
    memset(vec->end, 0, (vec->capacity - vec->size) * vec->itemSize);
}

void vectorShrinkToFit(Vector *vec)
{
    if (vec->size == 0) {
        free(vec->data);
        vec->end = vec->data = NULL;
        vec->capacity = 0;
    }
    else {
        void *tmp = realloc(vec->data, vec->itemSize * vec->size);
        if (tmp == NULL) {
            setError(ERR_Allocation);
            return;
        }

        vec->data = tmp;
        vec->end = vec->data + vec->size * vec->itemSize;
        vec->capacity = vec->size;
    }
}

uint32_t vectorCapacity(const Vector *vec)
{
    return vec->capacity;
}

uint32_t vectorSize(const Vector *vec)
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
