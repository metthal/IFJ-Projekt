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

/**
 * Initializes string to default size.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param  ps  String to initialize
 */
void initString(String *ps)
{
    _initStringSize(ps, STRING_DEFAULT_SIZE);
}

/**
 * Initializes string with size allocated specified by argument.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param  ps    String to initialize
 * @param  size  Size to allocate
 */
void initStringSize(String *ps, uint32_t size)
{
    _initStringSize(ps, size);
}

/**
 * Initializes string with C string.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param  ps   String to initialize
 * @param  str  Char buffer
 * @param  len  Length without \0
 */
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

/**
 * Initializes string with another string.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param  ps   String to initialize
 * @param  src  Source string
 */
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

/**
 * Frees buffer and resets members.
 *
 * @param  ps  String to delete
 */
void deleteString(String *ps)
{
    if (ps != NULL) {
        free(ps->data);
        ps->data = NULL;
        ps->size = 0;
        ps->length = 0;
    }
}

/**
 * Creates new String with default size.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @return  New String
 */
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

/**
 * Create new String with specified size.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param   size  Size to alloc
 *
 * @return        New String
 */
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

/**
 * Create new String from C string.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param   str   Char buffer
 * @param   len   Length without \0
 *
 * @return        New String
 */
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

/**
 * Frees whole string and sets pointer to NULL;
 *
 * @param  pps  Pointer to set
 */
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

/**
 * Clones string into new string.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param   ps  String to clone
 *
 * @return      New string
 */
String* stringClone(String *ps)
{
    String *nps = newStringSize(ps->size);
    if (nps) {
        nps->length = ps->length;
        memcpy(nps->data, ps->data, ps->length);
    }
    return nps;
}

/**
 * Returns new string from offset with length.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param   ps      Source String
 * @param   offset  Starts from 0
 * @param   length  Length of substring
 *
 * @return          New substring
 */
String* stringSubstr(const String *ps, uint32_t offset, uint32_t length)
{
    String *nps = newStringSize(length + 1);
    if (nps) {
        nps->length = length + 1;
        memcpy(nps->data, ps->data + offset, length);
        nps->data[length] = '\0';
    }
    return nps;
}

/**
 * Reduces string to substring.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param   ps      String to reduce
 * @param   offset  Starts from 0
 * @param   length  Length of substring
 */
void stringSubstrI(String *ps, uint32_t offset, uint32_t length)
{
    ps->length = length + 1;
    memmove(ps->data, ps->data + offset, length);
    ps->data[length] = '\0';
    if ((ps->length > STRING_DEFAULT_SIZE) &&
            (ps->length << STRING_RESIZE_DEC_RATE <= ps->size)) {
        _stringResizeRaw(ps, ps->length);
    }
}

/**
 * Empties string.
 *
 * @param  ps  String to empty
 */
void stringEmpty(String *ps)
{
    ps->data[0] = '\0';
    ps->length = 1;
}

/**
 * Pushes char to string.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param  ps  String to push into
 * @param  c   Char to push
 */
void stringPush(String *ps, char c)
{
    if (!(ps->length < ps->size)) {
        if (!_stringResizeRaw(ps, ps->size << STRING_RESIZE_INC_RATE)) {
            return;
        }
    }
    ps->data[ps->length - 1] = c;
    ps->data[ps->length] = '\0';
    ps->length++;
}

/**
 * Pops char from string.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param   ps  String to pop from
 *
 * @return      Popped char
 */
char stringPop(String *ps)
{
    char c = ps->data[ps->length - 2];
    ps->data[ps->length - 2] = '\0';
    ps->length--;
    if ((ps->length > STRING_DEFAULT_SIZE) &&
            (ps->length << STRING_RESIZE_DEC_RATE <= ps->size)) {
        _stringResizeRaw(ps, ps->length);
    }
    return c;
}

static inline void* _stringSetsizeRaw(String *dest, uint32_t size)
{
    free(dest->data);
    dest->data = malloc(sizeof(char) * size);
    dest->size = size;
    return dest->data;
}

/**
 * Copies value from source string to destination string.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param  src   Source string
 * @param  dest  Destination string
 */
void copyString(const String *src, String *dest)
{
    if ((dest->size < src->length)
        || ((src->length > STRING_DEFAULT_SIZE)
            && (dest->size >= src->length << STRING_RESIZE_DEC_RATE))) {
        if (!_stringSetsizeRaw(dest, src->length)) {
            setError(ERR_Allocation);
            return;
        }
    }
    memcpy(dest->data, src->data, src->length);
    dest->length = src->length;
}

/**
 * Sets string to C string value.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param  str   C string
 * @param  len   C string length
 * @param  dest  String to set
 */
void copyStringS(const char *str, uint32_t len, String *dest)
{
    uint32_t srcLen = len + 1;
    if ((dest->size < srcLen)
        || ((srcLen > STRING_DEFAULT_SIZE)
            && (dest->size >= srcLen << STRING_RESIZE_DEC_RATE))) {
        if (!_stringSetsizeRaw(dest, srcLen)) {
            setError(ERR_Allocation);
            return;
        }
    }
    memcpy(dest->data, str, len);
    dest->data[len] = '\0';
    dest->length = srcLen;
}

/**
 * Concatenates source string into destination string.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param  dest  Destination string
 * @param  src   Source string
 */
void stringAdd(String *dest, const String *src)
{
    uint32_t newLength = dest->length + src->length - 1;
    if (dest->size < newLength) {
        if (!_stringResizeRaw(dest, newLength)) {
            return;
        }
    }
    memcpy(dest->data + dest->length - 1, src->data, src->length - 1);
    dest->data[newLength - 1] = '\0';
    dest->length = newLength;
}

/**
 * Concatenates source C string into destination string.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param  dest  Destination string
 * @param  src   Source C string
 * @param  len   Source C string length
 */
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

/**
 * Concatenates source string into destination string,
 * so that source string precedes destination string.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param  dest  Destination string
 * @param  src   Source string
 */
void stringFrontAdd(String *dest, const String *src)
{
    // Calculate required space
    uint32_t newLength = dest->length + src->length - 1;
    // Create new memory place
    char *data = malloc(sizeof(char) * newLength);
    if (data == NULL) {
        setError(ERR_Allocation);
        return;
    }

    // Copy old data to offset
    memcpy(data + src->length - 1, dest->data, dest->length);
    // Copy new data to string beginning, without trailing zero
    memcpy(data, src->data, src->length - 1);

    free(dest->data);
    dest->data = data;
    dest->size = newLength;
    dest->length = newLength;
}

/**
 * Moves data of source string to destination string
 * without copying, leaving source string in uninitialized
 * state.
 *
 * @param  dest  Destination string
 * @param  src   Source string
 */
void stringMove(String *dest, String *src)
{
    deleteString(dest);
    dest->data = src->data;
    dest->length = src->length;
    dest->size = src->size;
    src->data = NULL;
    src->length = 0;
    src->size = 0;
}

/**
 * Returns string length without \0.
 *
 * @param   ps  String
 *
 * @return      String length
 */
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

/**
 * Compares two strings. Returns negative if b > a, zero if a = b, and positive if a > b.
 *
 * @param   a  String A
 * @param   b  String B
 *
 * @return     Result of comparison
 */
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
