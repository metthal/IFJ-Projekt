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

static inline ValueVectorIterator vectorFastAtValue(Vector *vec, uint32_t index)
{
    return ((Value*)vec->data) + index;
}

#endif
