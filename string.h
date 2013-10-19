#ifndef STRING_H
#define STRING_H

#include <ctype.h>
#include <stdint.h>

typedef struct {
    uint32_t size;
    uint32_t length;
    char *data;
} String;

static uint8_t const STRING_RESIZE_INC_RATE = 2;
static uint8_t const STRING_RESIZE_DEC_RATE = 3;
static uint16_t const STRING_DEFAULT_SIZE = 16;

void initString(String *ps);
void initStringSize(String *ps, uint32_t size);
void initStringS(String *ps, const char* str, uint32_t len);
void deleteString(String *ps);

String* newString();
String* newStringSize(uint32_t size);
String* newStringS(const char* str, uint32_t len);
void freeString(String **pps);

char* stringResize(String *ps, uint32_t size);
String* stringClone(String *ps);

void stringEmpty(String *ps);
void stringPush(String *ps, char c);
char stringPop(String *ps);
void stringCopy(String *src, String *dest);
void stringSet(String *dest, String *src);
void stringSetS(String *ps, const char *str, uint32_t len);
void stringAdd(String *dest, String *src);
void stringAddS(String *ps, const char *str, uint32_t len);

uint32_t stringLength(String *ps);

int16_t stringCompare(String *a, String *b);
int16_t stringCompareS(String *a, const char *b, uint32_t bLen);
int16_t stringCompareSS(const char *a, uint32_t aLen, const char *b, uint32_t bLen);

#endif
