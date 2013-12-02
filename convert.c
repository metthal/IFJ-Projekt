#include "convert.h"
#include "nierr.h"

#include <stdlib.h>

typedef enum
{
    CS_Start,
    CS_Number,
    CS_DecimalPoint,
    CS_Decimal,
    CS_Exponent,
    CS_ExponentSign
} ConvertState;

int stringToInt(const String *str)
{
    const char *directStr = str->data;
    while (isspace(*directStr))
        directStr++;

    // we don't support unary minus
    if (*directStr == '-')
        return 0;

    return strtol(directStr, NULL, 10);
}

double stringToDouble(const String *str)
{
    uint32_t charPos = 0;
    uint8_t state = CS_Start;
    while (charPos < str->length) {
        char currentChar = str->data[charPos++];

        switch (state) {
            case CS_Start:
                if (isspace(currentChar))
                    ;
                else if ('0' <= currentChar && currentChar <= '9')
                    state = CS_Number;
                else
                    return 0.0;
                break;

            case CS_Number:
                if ('0' <= currentChar && currentChar <= '9')
                    ;
                else if (currentChar == '.')
                    state = CS_DecimalPoint;
                else if (currentChar == 'e' || currentChar == 'E')
                    state = CS_Exponent;
                else
                    return strtod(str->data, NULL);
                break;

            case CS_DecimalPoint:
                if ('0' <= currentChar && currentChar <= '9')
                    state = CS_Decimal;
                else {
                    setError(ERR_Convert);
                    return 0.0;
                }
                break;

            case CS_Decimal:
                if ('0' <= currentChar && currentChar <= '9')
                    ;
                else if (currentChar == 'e' || currentChar == 'E')
                    state = CS_Exponent;
                else
                    return strtod(str->data, NULL);
                break;

            case CS_Exponent:
                // we are now completely sure that there is correct double number
                if ('0' <= currentChar && currentChar <= '9')
                    return strtod(str->data, NULL);
                else if (currentChar == '+' || currentChar == '-')
                    state = CS_ExponentSign;
                else {
                    setError(ERR_Convert);
                    return 0.0;
                }
                break;

            case CS_ExponentSign:
                // we are now completely sure that there is correct double number
                if ('0' <= currentChar && currentChar <= '9')
                    return strtod(str->data, NULL);
                else {
                    setError(ERR_Convert);
                    return 0.0;
                }
                break;
        }
    }

    return 0.0;
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
