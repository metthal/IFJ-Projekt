#ifndef RC_H
#define RC_H

#include "nierr.h"

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
    RC_FatalError       = 99,   ///< Error not related to interpreter (bad allocation etc)
    RC_Unknown          = 100   ///< @todo Remove this before submission, just internal
} ResultCode;

static inline ResultCode getRcFromError()
{
    switch (niErr.type) {
        case ERR_None:
            return RC_Ok;
        case ERR_LexFile:
            return RC_LexError;
        case ERR_NewFailed:
            return RC_FatalError;
        case ERR_Convert:
            return RC_NumCastError;
        default:
            return RC_Unknown;
    }
}

#endif
