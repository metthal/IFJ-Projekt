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
 * @file builtin.h
 * @brief Declares built-in functions and their helpers.
 */

#ifndef BUILTIN_H
#define BUILTIN_H

#include "string.h"
#include "symbol.h"

/** Codes for identifying builtin functions */
typedef enum
{
    BTI_None = 0,    //!< BTI_None
    BTI_BoolVal,     //!< BTI_BoolVal
    BTI_DoubleVal,   //!< BTI_DoubleVal
    BTI_FindString,  //!< BTI_FindString
    BTI_GetString,   //!< BTI_GetString
    BTI_GetSubstring,//!< BTI_GetSubstring
    BTI_IntVal,      //!< BTI_IntVal
    BTI_PutString,   //!< BTI_PutString
    BTI_SortString,  //!< BTI_SortString
    BTI_StrLen,      //!< BTI_StrLen
    BTI_StrVal,      //!< BTI_StrVal
} BuiltinCode;

/**
 * Returns built-in code when string matches a name
 * of built-in function. BTI_None otherwise.
 * @param str String to be matched.
 * @return Resulting built-in code.
 */
BuiltinCode getBuiltinCode(const String *str);

/**
 * Returns number of parameters for a built-in function.
 * @param code Built-in function code.
 * @return Number of parameters.
 */
int64_t getBuiltinParamCount(BuiltinCode code);

/**
 * @brief Converts value directly into boolean.
 * @param val Value to convert.
 * @return Resulting booelan.
 */
uint8_t valueToBool(const Value *val);

/**
 * @brief Converts value directly into integer.
 * @param val Value to convert.
 * @return Resulting integer.
 */
int valueToInt(const Value *val);

/**
 * @brief Converts value directly into double.
 * @param val Value to convert.
 * @return Resulting double.
 */
double valueToDouble(const Value *val);

/**
 * @brief Converts value directly into string.
 * @param val Value to convert.
 * @param str Resulting string.
 */
void valueToString(const Value *val, String *str);

/**
 * @brief Converts any value to boolean.
 * @param val Value to be converted.
 * @param ret Converted value.
 */
void boolval(const Value *val, Value *ret);

/**
 * @brief Converts any value to double.
 * @param val Value to be converted.
 * @param ret Converted value.
 */
void doubleval(const Value *val, Value *ret);

/**
 * Searches for a match of strings.
 * @param ret Index of first index in match as integer.
 * @param a String in which to search.
 * @param b String to be searched.
 */
void findString(Value *ret, const Value *a, const Value *b);

/**
 * Reads a string from the input.
 * @param ret Read string.
 */
void getString(Value *ret);

/**
 * @brief Cuts a substring out of original string.
 * @param val Original string. If not string, it is converted.
 * @param ret Resulting substring.
 * @param start First index to be included in substring.
 * @param end First index not to be included in substring.
 */
void getSubstring(const Value *val, Value *ret, int start, int end);

/**
 * @brief Converts any value to integer.
 * @param val Value to be converted.
 * @param ret Converted value.
 */
void intval(const Value *val, Value *ret);

/**
 * @brief Prints all passed values using predefined formating.
 * @param base Base stack address for offsetting.
 * @param constBase Base constants address for offsetting.
 * @param ret Number of printed values.
 * @param count Number of values passed.
 */
void putString(const Value *base, const Value *constBase, Value *ret, int count);

/**
 * @brief Sorts the passsed string.
 * @param val String to sort. If not string, it is converted.
 * @param ret Sorted string.
 */
void sortString(const Value *val, Value *ret);

/**
 * @brief Returns length of passed string.
 * @param val String to querry. If not string, it is converted.
 * @param ret Length of string as integer.
 */
void strLen(const Value *val, Value *ret);

/**
 * @brief Converts any value to string.
 * @param val Value to be converted.
 * @param ret Converted value.
 */
void strval(const Value *val, Value *ret);

#endif
