#include "convert.h"
#include "nierr.h"

#include <stdlib.h>

int stringToInt(const String *str)
{
    return strtol(str->data, NULL, 10);
}

double stringToDouble(const String *str)
{
    char *endptr = NULL;
    double tmp = strtod(str->data, &endptr);

    // error code is set only if it found as first non-digit character
    // in case *endptr == 0, we had just string full of whitespaces or empty one what means 0.0
    if (endptr == str->data) {
        // skip whitespaces at the beginning
        while (isspace(*endptr))
            endptr++;

        if (*endptr != '\0')
            setError(ERR_Convert);
    }

    return tmp;
}

String* intToString(int num)
{
    String *tmp = NULL;
    /* Buffer size explanation.
     * Unsigned:
     * log10(2^64) = 19.2 => 20 + 1 (null terminator)
     * Signed:
     * log10(2^63) = 18.9 => 19 + 1 (sign) + 1 (null terminator)
     * 21 chars needed at most.
     */
    char buffer[21];
    int written = sprintf(buffer, "%d", num);
    if (written > 0)
        tmp = newStringS(buffer, written);
    else
        tmp = newString();
    return tmp;
}

// String shouldn't be initialized
int intToStringE(int num, String *str)
{
    // See intToString for explanation
    char buffer[21];
    int written = sprintf(buffer, "%d", num);
    if (str != NULL) {
        if (written > 0)
            initStringS(str, buffer, written);
        else
            initString(str);
    }
    return written;
}

String* doubleToString(double num)
{
    String *tmp = NULL;
    /* Buffer size explanation.
     * Sign: 1
     * Mantisa: 15-17 digits => 17 + 1 (dot)
     * Exponent: (1 (e / E) + 1 (sign) + log10(2^10)) = 6
     * Null terminator: 1
     * 26 chars needed at most.
     */
    char buffer[26];
    int written = sprintf(buffer, "%g", num);
    if (written > 0)
        tmp = newStringS(buffer, written);
    else
        tmp = newString();
    return tmp;
}

// String shouldn't be initialized
int doubleToStringE(double num, String *str)
{
    // See doubleToString for explanation
    char buffer[26];
    int written = sprintf(buffer, "%g", num);
    if (str != NULL) {
        if (written > 0)
            initStringS(str, buffer, written);
        else
            initString(str);
    }
    return written;
}
