#include <stdlib.h>
#include <stdio.h>

#include "ial.h"

uint32_t* stringSubstrSearchBuildTable(const char *str, uint32_t len)
{
    uint32_t *table = malloc(sizeof(uint32_t) * len);
    if (!table) {
        return table;
    }
    uint32_t tableIndex = 2, jumpIndex = 0;
    table[0] = table[1] = 0;

    while (tableIndex < len) {
        if (str[tableIndex - 1] == str[jumpIndex]) {
            table[tableIndex] = ++jumpIndex;
            tableIndex++;
        } else if (jumpIndex > 0) {
            jumpIndex = table[jumpIndex];
        } else {
            table[tableIndex] = 0;
            tableIndex++;
        }
    }

    return table;
}

static inline uint32_t _stringSubstrSearchSSO(const char *haystack, uint32_t haystackLen, const char *needle, uint32_t needleLen, uint32_t offset)
{
    uint32_t haystackIndex = offset, needleIndex = 0, result = haystackLen;
    uint32_t *table = stringSubstrSearchBuildTable(needle, needleLen);

    while (haystackIndex + needleIndex < haystackLen) {
        if (needle[needleIndex] == haystack[haystackIndex + needleIndex]) {
            if (needleIndex == needleLen - 1) {
                result = haystackIndex;
                break;
            }
            needleIndex++;
        } else {
            if (needleIndex > 0) {
                haystackIndex += needleIndex - table[needleIndex];
                needleIndex = table[needleIndex];
            } else {
                haystackIndex += needleIndex + 1;
            }
        }
    }
    
    free(table);
    return result;
}

uint32_t stringSubstrSearch(String *haystack, String *needle)
{
    return _stringSubstrSearchSSO(haystack->data, haystack->length - 1, needle->data, needle->length - 1, 0);
}

uint32_t stringSubstrSearchO(String *haystack, String *needle, uint32_t offset)
{
    return _stringSubstrSearchSSO(haystack->data, haystack->length - 1, needle->data, needle->length - 1, offset);
}

uint32_t stringSubstrSearchS(String *haystack, const char *needle, uint32_t needleLen)
{
    return _stringSubstrSearchSSO(haystack->data, haystack->length - 1, needle, needleLen, 0);
}

uint32_t stringSubstrSearchSO(String *haystack, const char *needle, uint32_t needleLen, uint32_t offset)
{
    return _stringSubstrSearchSSO(haystack->data, haystack->length - 1, needle, needleLen, offset);
}

uint32_t stringSubstrSearchSS(const char *haystack, uint32_t haystackLen, const char *needle, uint32_t needleLen)
{
    return _stringSubstrSearchSSO(haystack, haystackLen, needle, needleLen, 0);
}

uint32_t stringSubstrSearchSSO(const char *haystack, uint32_t haystackLen, const char *needle, uint32_t needleLen, uint32_t offset)
{
    return _stringSubstrSearchSSO(haystack, haystackLen, needle, needleLen, offset);
}
