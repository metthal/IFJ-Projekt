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
