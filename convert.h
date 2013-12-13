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

/**
 * @file convert.h
 * @brief Declares internal conversion functions.
 */

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
