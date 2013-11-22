#ifndef CONVERT_H
#define CONVERT_H

#include <stdint.h>
#include "string.h"

// No format checks are performed in functions below.

/** Converts string to integer.
 * @param str String to convert.
 * @return Integer representation of the string.
 */
int stringToInt(const String *str);

/** Converts string to double.
 * @param str String to convert.
 * @return Double representation of the string.
 */
double stringToDouble(const String *str);

/** Converts integer to string.
 * @param num Integer to convert.
 * @return New instance of string object with converted value.
 */
String* intToString(int num);

/** Converts integer to existing string.
 * @param num Integer to convert.
 * @param str Initialized string that will hold converted value.
 * @return Length of resulting string.
 */
int intToStringE(int num, String *str);

/** Converts double to string.
 * @param num Double to convert.
 * @return New instance of string object with converted value.
 */
String* doubleToString(double num);

/** Converts double to existing string.
 * @param num Double to convert.
 * @param str Initialized string that will hold converted value.
 * @return Length of resulting string.
 */
int doubleToStringE(double num, String *str);

#endif
