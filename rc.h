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
 * @file rc.h
 * @author INI Team
 *
 * @brief Result codes and everything about application termination
 **/
#ifndef RC_H
#define RC_H

#include "nierr.h"

/**
 * @enum ResultCode
 *
 * Result or exit codes of the application as defined by assignment
 *
 * @todo Remove @ref RC_Unknown before submission, just internal
 **/
typedef enum
{
    RC_Ok               = 0,    ///< Successful intepretation
    RC_LexError         = 1,    ///< Lexical analysis error
    RC_SynError         = 2,    ///< Syntax analysis error
    RC_FuncDefError     = 3,    ///< Semantic error - function undefined / redefined
    RC_FuncParamError   = 4,    ///< Semantic/runtime error - wrong parameters on function call
    RC_VarError         = 5,    ///< Semantic/runtime error - variable not declared
    RC_DivByZero        = 10,   ///< Semantic/runtime error - division by zero
    RC_NumCastError     = 11,   ///< Semantic/runtime error - error in casting to number (function doubleval)
    RC_ArithmError      = 12,   ///< Semantic/runtime error - type compatibility in arithmetic expressions
    RC_UnkError         = 13,   ///< Semantic/runtime error - Other errors
    RC_FatalError       = 99    ///< Error not related to interpreter (bad allocation etc)
} ResultCode;

/**
 * Transforms @ref NiErrorType into @ref ResultCode
 *
 * @return Result code of corresponding error type
 **/
static inline ResultCode getRcFromError()
{
    switch (niErr.type) {
        case ERR_None:
            return RC_Ok;

        case ERR_LexFile:
            return RC_LexError;

        case ERR_Syntax:
        case ERR_CycleControl:
        case ERR_BadDefArg:
            return RC_SynError;

        case ERR_Allocation:
            return RC_FatalError;

        case ERR_Convert:
            return RC_NumCastError;

        case ERR_UndefFunction:
        case ERR_RedefFunction:
            return RC_FuncDefError;

        case ERR_UndefVariable:
            return RC_VarError;

        case ERR_BadParamCount:
        case ERR_DefArgOrder:
            return RC_FuncParamError;

        case ERR_RedefParameter:
        case ERR_Substr:
            return RC_UnkError;

        case ERR_OperandTypes:
            return RC_ArithmError;

        case ERR_DivideByZero:
            return RC_DivByZero;

        default:
            return RC_FatalError;
    }
}

#endif
