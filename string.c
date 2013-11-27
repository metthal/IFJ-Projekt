#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "string.h"
#include "nierr.h"

static inline void* _initStringSize(String *ps, uint32_t size)
{
    ps->data = malloc(sizeof(char) * size);
    if (ps->data != NULL) {
        ps->data[0] = '\0';
        ps->size = size;
        ps->length = 1;
    } else {
        setError(ERR_Allocation);
    }
    return ps->data;
}

void initString(String *ps)
{
    _initStringSize(ps, STRING_DEFAULT_SIZE);
}

void initStringSize(String *ps, uint32_t size)
{
    _initStringSize(ps, size);
}

void initStringS(String *ps, const char *str, uint32_t len)
{
    ps->size = len + 1;
    ps->length = len + 1;
    ps->data = malloc(sizeof(char) * (len + 1));
    if (ps->data != NULL) {
        memcpy(ps->data, str, len);
        ps->data[len] = '\0';
    } else {
        setError(ERR_Allocation);
    }
}

void initStringSet(String *ps, const String *src)
{
    ps->size = src->size;
    ps->length = src->length;
    ps->data = malloc(sizeof(char) * src->size);
    if (ps->data != NULL) {
        memcpy(ps->data, src->data, sizeof(char) * src->length);
    } else {
        setError(ERR_Allocation);
    }
}

void deleteString(String *ps)
{
    if (ps != NULL) {
        free(ps->data);
        ps->data = NULL;
        ps->size = 0;
        ps->length = 0;
    }
}

String* newString()
{
    String *ps = malloc(sizeof(String));
    if (ps) {
        if (!_initStringSize(ps, STRING_DEFAULT_SIZE)) {
            free(ps);
            ps = NULL;
        }
    }
    else {
        setError(ERR_Allocation);
    }
    return ps;
}

String* newStringSize(uint32_t size)
{
    String *ps = malloc(sizeof(String));
    if (ps != NULL) {
        if (!_initStringSize(ps, size)) {
            free(ps);
            ps = NULL;
        }
    }
    else {
        setError(ERR_Allocation);
    }
    return ps;
}

String* newStringS(const char *str, uint32_t len)
{
    String *ps = malloc(sizeof(String));
    if (ps != NULL) {
        ps->size = len + 1;
        ps->length = len + 1;
        ps->data = malloc(sizeof(char) * (len + 1));
        if (ps->data != NULL) {
            memcpy(ps->data, str, len);
            ps->data[len] = '\0';
        } else {
            free(ps);
            ps = NULL;
            setError(ERR_Allocation);
        }
    }
    else {
        setError(ERR_Allocation);
    }
    return ps;
}

void freeString(String **pps) {
    if (pps != NULL) {
        if (*pps != NULL) {
            free((*pps)->data);
        }
        free(*pps);
        *pps = NULL;
    }
}

static inline void* _stringResizeRaw(String *ps, uint32_t size)
{
    ps->data = realloc(ps->data, size);
    if (!ps->data) {
        setError(ERR_Allocation);
        return NULL;
    }
    ps->size = size;
    return ps->data;
}

/**
 * Forces string to resize.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param   ps    String to resize
 * @param   size  Size
 */
void stringResize(String *ps, uint32_t size)
{
    if (!_stringResizeRaw(ps, size)) {
        return;
    }
    if (ps->length > ps->size) {
        ps->length = ps->size;
        ps->data[ps->length - 1] = '\0';
    }
}

String* stringClone(String *ps)
{
    String *nps = newStringSize(ps->size);
    if (nps) {
        nps->length = ps->length;
        memcpy(nps->data, ps->data, ps->length);
    }
    return nps;
}

String* stringSubstr(const String *ps, uint32_t offset, uint32_t length)
{
    String *nps = newStringSize(ps->size);
    if (nps) {
        nps->length = length + 1;
        memcpy(nps->data, ps->data + offset, length);
        nps->data[length] = '\0';
    }
    return nps;
}

void stringEmpty(String *ps)
{
    ps->data[0] = '\0';
    ps->length = 1;
}

void stringPush(String *ps, char c)
{
    if (!(ps->length < ps->size)) {
        if (!_stringResizeRaw(ps, STRING_RESIZE_INC_RATE * ps->size)) {
            return;
        }
    }
    ps->data[ps->length - 1] = c;
    ps->data[ps->length] = '\0';
    ps->length++;
}

char stringPop(String *ps)
{
    char c = ps->data[ps->length - 2];
    ps->data[ps->length - 2] = '\0';
    ps->length--;
    if (ps->length > STRING_DEFAULT_SIZE) {
        if (ps->length <= ps->size / STRING_RESIZE_DEC_RATE) {
            _stringResizeRaw(ps, ps->length);
        }
    }
    return c;
}

void stringCopy(const String *src, String *dest)
{
    if (!_stringResizeRaw(dest, src->length)) {
        return;
    }
    memcpy(dest->data, src->data, src->length);
    dest->length = src->length;
}

void stringSet(String *dest, String *src)
{
    if (!_stringResizeRaw(dest, src->length)) {
        return;
    }
    memcpy(dest->data, src->data, src->length);
    dest->length = src->length;
}

void stringSetS(String *dest, const char *str, uint32_t len)
{
    if (!_stringResizeRaw(dest, len + 1)) {
        return;
    }
    memcpy(dest->data, str, len);
    dest->data[len] = '\0';
    dest->length = len + 1;
}

void stringAdd(String *dest, String *src)
{
    uint32_t newLength = dest->length + src->length - 1;
    if (dest->size < newLength) {
        if (!_stringResizeRaw(dest, newLength)) {
            return;
        }
    }
    memcpy(dest->data + dest->length - 1, src->data, src->length);
    dest->length = newLength;
}

void stringAddS(String *dest, const char *src, uint32_t len)
{
    uint32_t newLength = dest->length + len;
    if (dest->size < newLength) {
        if (!_stringResizeRaw(dest, newLength)) {
            return;
        }
    }
    memcpy(dest->data + dest->length - 1, src, len);
    dest->length = newLength;
    dest->data[dest->length - 1] = '\0';
}

uint32_t stringLength(const String *ps)
{
    return ps->length - 1;
}

static inline int16_t _stringCompareSS(const char *a, uint32_t aLen, const char *b, uint32_t bLen)
{
    if (aLen < bLen) {
        return -b[aLen];
    }
    else if (aLen > bLen) {
        return a[bLen];
    }
    const unsigned char *s1 = (unsigned char*)a, *s2 = (unsigned char*)b;
    while (*s1 && (*s1 == *s2)) {
        s1++,s2++;
    }
    return *s1 - *s2;
}

int16_t stringCompare(const String *a, const String *b)
{
    return _stringCompareSS(a->data, a->length - 1, b->data, b->length - 1);
}

int16_t stringCompareS(const String *a, const char *b, uint32_t bLen)
{
    return _stringCompareSS(a->data, a->length - 1, b, bLen);
}

int16_t stringCompareSS(const char *a, uint32_t aLen, const char *b, uint32_t bLen)
{
    return _stringCompareSS(a, aLen, b, bLen);
}
