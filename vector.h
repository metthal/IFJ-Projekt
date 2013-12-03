#ifndef VECTOR_H
#define VECTOR_H

#include <stdint.h>
#include <stdlib.h>

typedef struct
{
    uint32_t size; ///< Used size of data container.
    uint32_t capacity; ///< Real size of data container.
    size_t itemSize; ///< Size of single item in bytes.
    uint8_t *data; ///< Data container.
    uint8_t *end; ///< Access optimization member.
} Vector;

/**
 * Reserves specified capacity.
 * Useful when final number of items is known to prevent
 * multiple expensive reallocations.
 * Sets ERR_Allocation if requested space
 * couldn't be allocated and preserves previous capacity
 * and data content.
 * Note: Supports only increase in capacity.
 * @param vec Vector to reserve memory in.
 * @param capacity New capacity of vector.
 */
void vectorReserve(Vector *vec, uint32_t capacity);

/**
 * Shrinks vector capacity to accommodate only
 * current size.
 * Useful after finalizing vector changes to
 * free unused space, but requires reallocation.
 * Sets ERR_Allocation if reallocation to new block
 * failed and preserves previous capacity and data content.
 * @param vec Vector to shrink.
 */
void vectorShrinkToFit(Vector *vec);

/**
 * Returns current vector capacity.
 * @param vec Vector to query.
 * @return Capacity of vector.
 */
uint32_t vectorCapacity(const Vector *vec);

/**
 * Returns current vector size.
 * @param vec Vector to query.
 * @return Size of vector.
 */
uint32_t vectorSize(const Vector *vec);

/**
 * Returns pointer to item at specified index.
 * When index is out of vector range, ERR_Range
 * error is set.
 * @param vec Vector to operate on.
 * @param index Index of item in vector.
 * @return Pointer to indexed item.
 */
void* vectorAt(Vector *vec, uint32_t index);

/**
 * Returns constant pointer to item at specified index.
 * When index is out of vector range, ERR_Range
 * error is set.
 * @param vec Constant vector to operate on.
 * @param index Index of item in vector.
 * @return Constant pointer to indexed item.
 */
const void* vectorAtConst(const Vector *vec, uint32_t index);

/**
 * Returns pointer to first item in vector.
 * When vector is empty, ERR_Range error is set.
 * @param vec Vector to operate on.
 * @return Pointer to first item.
 */
void* vectorFront(Vector *vec);

/**
 * Returns pointer to last item in vector.
 * When vector is empty, ERR_Range error is set.
 * @param vec Vector to operate on.
 * @return Pointer to last item.
 */
void* vectorBack(Vector *vec);

#endif
