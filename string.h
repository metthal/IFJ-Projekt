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

#ifndef STRING_H
#define STRING_H

#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "nierr.h"

// Shortcut for declaring str in arguments
#define CSTR_ARG(str)   str, sizeof(str) - 1

typedef struct
{
    uint32_t size;
    uint32_t length;
    char *data;
} String;

static uint8_t const STRING_RESIZE_INC_RATE = 1;
static uint8_t const STRING_RESIZE_DEC_RATE = 2;
static uint16_t const STRING_DEFAULT_SIZE = 16;

void initString(String *ps);
void initStringSize(String *ps, uint32_t size);
void initStringS(String *ps, const char *str, uint32_t len);

/**
 * Initializes string with another string.
 * Sets ERR_Allocation if memory couldn't be allocated.
 *
 * @param  ps   String to initialize
 * @param  src  Source string
 */
static inline void initStringSet(String *ps, const String *src)
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
static inline void deleteString(String *ps)
{
    if (ps != NULL) {
        free(ps->data);
        ps->data = NULL;
        ps->size = 0;
        ps->length = 0;
    }
}

String* newString();
String* newStringSize(uint32_t size);
String* newStringS(const char *str, uint32_t len);
void freeString(String **pps);

void stringResize(String *ps, uint32_t size);
String* stringClone(String *ps);
String* stringSubstr(const String *ps, uint32_t offset, uint32_t length);
void stringSubstrI(String *ps, uint32_t offset, uint32_t length);

void stringEmpty(String *ps);
void stringPush(String *ps, char c);
char stringPop(String *ps);
void copyString(const String *src, String *dest);
void copyStringS(const char *src, uint32_t len, String *dest);
void stringAdd(String *dest, const String *src);
void stringAddS(String *dest, const char *src, uint32_t len);
void stringFrontAdd(String *dest, const String *src);

void stringMove(String *dest, String *src);

uint32_t stringLength(const String *ps);

int16_t stringCompare(const String *a, const String *b);
int16_t stringCompareS(const String *a, const char *b, uint32_t bLen);
int16_t stringCompareSS(const char *a, uint32_t aLen, const char *b, uint32_t bLen);

#endif
