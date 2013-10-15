#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "string.h"

#define STRING_RESIZE_RAW(STRING, SIZE) \
        STRING->data = realloc(STRING->data, SIZE); \
        STRING->size = SIZE;

String* newString()
{
    String *ps = malloc(sizeof(String));
    if (ps != NULL) {
        ps->size = STRING_DEFAULT_SIZE;
        ps->length = 1;
        ps->data = malloc(sizeof(char) * STRING_DEFAULT_SIZE);
        if (ps->data != NULL) {
            ps->data[0] = '\0';
        } else {
            free(ps);
            ps = NULL;
        }
    }
    return ps;
}

String* newStringSize(uint32_t size)
{
    String *ps = malloc(sizeof(String));
    if (ps != NULL) {
        ps->size = size;
        ps->length = 1;
        ps->data = malloc(sizeof(char) * size);
        if (ps->data != NULL) {
            ps->data[0] = '\0';
        } else {
            free(ps);
            ps = NULL;
        }
    }
    return ps;
}

String* newStringS(const char *str, uint32_t len)
{
    if (str[len - 1] == '\0') {
        len--;
    }
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
        }
    }
    return ps;
}

void freeString(String **pps) {
    if (pps) {
        if (*pps) {
            free((*pps)->data);
        }
        free(*pps);
        *pps = NULL;
    }
}

char* stringResize(String *ps, uint32_t size)
{
    STRING_RESIZE_RAW(ps, size);
    if (ps->length > ps->size) {
        ps->length = ps->size;
        ps->data[ps->length - 1] = '\0';
    }
    return ps->data;
}

String* stringClone(String *ps)
{
    String *nps = newStringSize(ps->size);
    if (nps != NULL) {
        nps->length = ps->length;
        memcpy(nps->data, ps->data, ps->length);
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
        uint32_t newSize = STRING_RESIZE_INC_RATE * ps->size;
        STRING_RESIZE_RAW(ps, newSize);
    }
    ps->data[ps->length - 1] = c;
    ps->data[ps->length] = '\0';
    ps->length++;
}

char stringPop(String *ps)
{
    if (ps->length <= 1) {
        return 0;
    }
    char c = ps->data[ps->length - 2];
    ps->data[ps->length - 2] = '\0';
    ps->length--;
    if (ps->length <= ps->size / STRING_RESIZE_DEC_RATE) {
        STRING_RESIZE_RAW(ps, ps->length);
    }
    return c;
}

void stringCopy(String *src, String *dest)
{
    STRING_RESIZE_RAW(dest, src->length);
    memcpy(dest->data, src->data, src->length);
    dest->length = src->length;
}

void stringSet(String *dest, String *src)
{
    STRING_RESIZE_RAW(dest, src->length);
    memcpy(dest->data, src->data, src->length);
    dest->length = src->length;
}

void stringSetS(String *dest, const char *str, uint32_t len)
{
    if (str[len - 1] == '\0') {
        len--;
    }
    STRING_RESIZE_RAW(dest, len + 1);
    memcpy(dest->data, str, len);
    dest->data[len] = '\0';
    dest->length = len + 1;
}

void stringAdd(String *dest, String *src)
{
    uint32_t newLength = dest->length + src->length - 1;
    if (dest->size < newLength) {
        STRING_RESIZE_RAW(dest, newLength);
    }
    memcpy(dest->data + dest->length - 1, src->data, src->length);
    dest->length = newLength;
}

void stringAddS(String *dest, const char *src, uint32_t len)
{
    if (src[len - 1] == '\0') {
        len--;
    }
    uint32_t newLength = dest->length + len;
    if (dest->size < newLength) {
        STRING_RESIZE_RAW(dest, newLength);
    }
    memcpy(dest->data + dest->length - 1, src, len);
    dest->length = newLength;
    dest->data[dest->length - 1] = '\0';
}

uint32_t stringLength(String *ps)
{
    return ps->length - 1;
}

