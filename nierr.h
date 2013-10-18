#ifndef NIERR_H
#define NIERR_H

#include <stdio.h>

/*
 * Add new values to enum as needed, but remember that those are error
 * codes of interpreter thus at higher abstraction level. E.g. we don't need
 * information that file doesn't exist, just that lexical analyzer had problem
 * related to file, which can also be caused by missing <?php tag etc.
 */

typedef enum
{
    ERR_None = 0,
    ERR_Unknown,
    ERR_LexFile,
    ERR_NewFailed,
    ERR_Convert
} NiErrorType;

typedef struct
{
    NiErrorType type;
    int line;
    const char *file;
    const char *fun;
} NiError;

extern NiError niErr;

void printError();

/** Gets last interpret error.
 * @return Last error.
 */
static inline NiErrorType getError()
{
#ifdef DEBUG
    printError();
#endif
    return niErr.type;
}

/** Sets interpret error.
 * @param err One of NiErrorType values.
 */
#define setError(err) setErrorExp(err, __LINE__, __FILE__, __func__)
static inline void setErrorExp(NiErrorType errType, int line, const char *file,
        const char *fun)
{
    niErr.type = errType;
    niErr.line = line;
    niErr.file = file;
    niErr.fun = fun;
}

/** Clears interpret error to ERR_None */
static inline void clearError()
{
    niErr.type = ERR_None;
}
#endif
