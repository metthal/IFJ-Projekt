#include "builtin.h"
#include "convert.h"
#include "nierr.h"
#include "ial.h"

#include <stdio.h>

#define READ_BUFFER_SIZE 64

uint8_t valueToBool(const Value *val)
{
    switch (val->type) {
        case VT_Null:
            return 0;

        case VT_Bool:
            return val->data.b;

        case VT_Integer:
            return val->data.i != 0;

        case VT_Double:
            return val->data.d != 0.0;

        case VT_String:
            return stringLength(&(val->data.s)) != 0;

        case VT_Undefined:
            setError(ERR_UndefVariable);
            return 0;

        case VT_StackPtr:
        case VT_InstructionPtr:
            setError(ERR_Internal);
            return 0;
    }
    return 0;
}

int valueToInt(const Value *val)
{
    switch (val->type) {
        case VT_Null:
            return 0;

        case VT_Bool:
            return (int32_t)val->data.b;

        case VT_Integer:
            return val->data.i;

        case VT_Double:
            return (int32_t)val->data.d;

        case VT_String:
            return stringToInt(&(val->data.s));

        case VT_Undefined:
            setError(ERR_UndefVariable);
            return 0;

        case VT_StackPtr:
        case VT_InstructionPtr:
            setError(ERR_Internal);
            return 0;
    }
    return 0;
}

void valueToString(const Value *val, String *str)
{
    switch (val->type) {
        case VT_Null:
            initString(str);
            break;

        case VT_Bool:
            if (val->data.b)
                initStringS(str, "1", 1);
            else
                initString(str);
            break;

        case VT_Integer:
            intToStringE(val->data.i, str);
            break;

        case VT_Double:
            doubleToStringE(val->data.d, str);
            break;

        case VT_String:
            initStringSet(str, &(val->data.s));
            break;

        case VT_Undefined:
            setError(ERR_UndefVariable);
            return;

        case VT_StackPtr:
        case VT_InstructionPtr:
            setError(ERR_Internal);
            return;
    }
}

void boolval(const Value *val, Value *ret)
{
    ret->data.b = valueToBool(val);
    ret->type = VT_Bool;
}

void doubleval(const Value *val, Value *ret)
{
    switch (val->type) {
        case VT_Null:
            ret->data.d = 0.0;
            break;

        case VT_Bool:
            ret->data.d = (double)val->data.b;
            break;

        case VT_Integer:
            ret->data.d = (double)val->data.i;
            break;

        case VT_Double:
            ret->data.d = val->data.d;
            break;

        case VT_String:
            ret->data.d = stringToDouble(&(val->data.s));
            break;

        case VT_Undefined:
            setError(ERR_UndefVariable);
            return;

        case VT_StackPtr:
        case VT_InstructionPtr:
            setError(ERR_Internal);
            return;
    }
    ret->type = VT_Double;
}

void findString(Value *ret, const Value *a, const Value *b)
{
    String c, d;
    const String *x = NULL, *y = NULL;

    switch (a->type) {
        case VT_Null:
            initString(&c);
            x = &c;
            break;

        case VT_Bool:
            if (a->data.b)
                initStringS(&c, "1", 1);
            else
                initString(&c);
            x = &c;
            break;

        case VT_Integer:
            intToStringE(a->data.i, &c);
            x = &c;
            break;

        case VT_Double:
            doubleToStringE(a->data.d, &c);
            x = &c;
            break;

        case VT_String:
            x = &(a->data.s);
            break;

        case VT_Undefined:
            setError(ERR_UndefVariable);
            return;

        case VT_StackPtr:
        case VT_InstructionPtr:
            setError(ERR_Internal);
            return;
    }

    if (getError())
        return;

    switch (b->type) {
        case VT_Null:
            initString(&d);
            y = &d;
            break;

        case VT_Bool:
            if (b->data.b)
                initStringS(&d, "1", 1);
            else
                initString(&d);
            y = &d;
            break;

        case VT_Integer:
            intToStringE(b->data.i, &d);
            y = &d;
            break;

        case VT_Double:
            doubleToStringE(b->data.d, &d);
            y = &d;
            break;

        case VT_String:
            y = &(b->data.s);
            break;

        case VT_Undefined:
            setError(ERR_UndefVariable);
            // break because cleanup is required
            break;

        case VT_StackPtr:
        case VT_InstructionPtr:
            setError(ERR_Internal);
            // break because cleanup is required
            break;
    }

    if (getError()) {
        if (x == &c)
            deleteString(&c);
        return;
    }

    ret->type = VT_Integer;
    ret->data.i = stringSubstrSearch(x, y);

    if (x == &c)
        deleteString(&c);

    if (y == &d)
        deleteString(&d);
}

void getString(Value *ret)
{
    char buffer[READ_BUFFER_SIZE];
    String *workingStr = &(ret->data.s);
    initString(workingStr);
    ret->type = VT_String;

    // Buffered read until EOL or EOF
    int c;
    int i = 0;
    for (; (c = getchar()) && (c != '\n' && c != EOF); i++) {
        if (i == READ_BUFFER_SIZE) {
            stringAddS(workingStr, buffer, i);
            i = 0;
        }
        buffer[i] = c;
    }

    if (i > 0)
        stringAddS(workingStr, buffer, i);
}

void getSubstring(const Value *val, Value *ret, int start, int end)
{
    if(start > end || start < 0 || end < 0) {
        setError(ERR_Substr);
        return;
    }

    int len = -1;
    String str;
    const String *workingStr = NULL;

    switch (val->type) {
        case VT_Null:
            len = 0;
            break;

        case VT_Bool:
            if (val->data.b) {
                initStringS(&str, "1", 1);
                workingStr = &str;
                len = 1;
                return;
            }
            else
                len = 0;
            break;

        case VT_Integer: {
            len = intToStringE(val->data.i, &str);
            workingStr = &str;
            break;
        }

        case VT_Double: {
            len = doubleToStringE(val->data.d, &str);
            workingStr = &str;
            break;
        }

        case VT_String:
            workingStr = &(val->data.s);
            len = stringLength(workingStr);
            break;

        case VT_Undefined:
            setError(ERR_UndefVariable);
            return;

        case VT_StackPtr:
        case VT_InstructionPtr:
            setError(ERR_Internal);
            return;
    }

    if (start >= len || end > len) {
        setError(ERR_Substr);
        return;
    }

    if (!getError()) {
        // TODO optimize
        String *substr = stringSubstr(workingStr, start, end - start);
        stringCopy(substr, &(ret->data.s));
        freeString(&substr);
        ret->type = VT_String;
    }

    if (workingStr == &str)
        deleteString(&str);
}

void intval(const Value *val, Value *ret)
{
    ret->data.i = valueToInt(val);
    ret->type = VT_Integer;
}

void putString(Value *ret, Value *firstVal, int count)
{
    int i = 0;
    for (;i < count; i++, firstVal++){
        switch (firstVal->type) {
            case VT_Null:
                break;

            case VT_Bool:
                if (firstVal->data.b)
                    putchar('1');
                break;

            case VT_Integer:
                printf("%d", firstVal->data.i);
                break;

            case VT_Double:
                printf("%g", firstVal->data.d);
                break;

            case VT_String:
                printf("%s", firstVal->data.s.data);
                break;

            case VT_Undefined:
                setError(ERR_UndefVariable);
                return;

            case VT_StackPtr:
            case VT_InstructionPtr:
                setError(ERR_Internal);
                return;
        }
    }

    ret->type = VT_Integer;
    ret->data.i = i;

    // Force printing to output
    fflush(stdout);
}

void sortString(const Value *val, Value *ret)
{
    switch (val->type) {
        case VT_Null:
            initString(&(ret->data.s));
            break;

        case VT_Bool:
            if (val->data.b)
                initStringS(&(ret->data.s), "1", 1);
            else
                initString(&(ret->data.s));
            break;

        case VT_Integer: {
            intToStringE(val->data.i, &(ret->data.s));
            stringCharSort(&(val->data.s));
            break;
        }

        case VT_Double: {
            doubleToStringE(val->data.d, &(ret->data.s));
            stringCharSort(&(val->data.s));
            break;
        }

        case VT_String:
            initStringSet(&(ret->data.s), &(val->data.s));
            stringCharSort(&(ret->data.s));
            break;

        case VT_Undefined:
            setError(ERR_UndefVariable);
            return;

        case VT_StackPtr:
        case VT_InstructionPtr:
            setError(ERR_Internal);
            return;
    }
    ret->type = VT_String;
}

void strLen(const Value *val, Value *ret)
{
    switch (val->type) {
        case VT_Null:
            ret->data.i = 0;
            break;

        case VT_Bool:
            // True => "1", False => ""
            ret->data.i = val->data.b ? 1 : 0;
            break;

        case VT_Integer:
            ret->data.i = intToStringE(val->data.i, NULL);
            break;

        case VT_Double:
            ret->data.i = doubleToStringE(val->data.d, NULL);
            break;

        case VT_String:
            ret->data.i = stringLength(&(val->data.s));
            break;

        case VT_Undefined:
            setError(ERR_UndefVariable);
            return;

        case VT_StackPtr:
        case VT_InstructionPtr:
            setError(ERR_Internal);
            return;
    }
    ret->type = VT_Integer;
}

void strval(const Value *val, Value *ret)
{
    valueToString(val, &(ret->data.s));
    ret->type = VT_String;
}
