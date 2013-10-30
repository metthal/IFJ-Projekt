#ifndef IAL_H
#define IAL_H

#include <ctype.h>
#include <stdint.h>

#include "string.h"

uint32_t stringSubstrSearch(String *haystack, String *needle);
uint32_t stringSubstrSearchO(String *haystack, String *needle, uint32_t offset);
uint32_t stringSubstrSearchS(String *haystack, const char *needle, uint32_t needleLen);
uint32_t stringSubstrSearchSO(String *haystack, const char *needle, uint32_t needleLen, uint32_t offset);
uint32_t stringSubstrSearchSS(const char *haystack, uint32_t haystackLen, const char *needle, uint32_t needleLen);
uint32_t stringSubstrSearchSSO(const char *haystack, uint32_t haystackLen, const char *needle, uint32_t needleLen, uint32_t offset);

#endif
