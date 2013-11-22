#include "builtin.h"
#include "convert.h"
#include "nierr.h"

#include <stdio.h>

#define READ_BUFFER_SIZE 64

void boolval(const Value *val, Value *ret)
{
    switch (val->type) {
        case VT_Null:
            ret->data.b = 0;
            break;

        case VT_Bool:
            ret->data.b = val->data.b;
            break;

        case VT_Integer:
            ret->data.b = val->data.i != 0;
            break;

        case VT_Double:
            ret->data.b = val->data.d != 0.0;
            break;

        case VT_String:
            ret->data.b = stringLength(&(ret->data.s)) != 0;
            break;

        case VT_Undefined:
            setError(ERR_UndefVariable);
            return;

        case VT_StackPtr:
        case VT_InstructionPtr:
            setError(ERR_Internal);
            return;
    }
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
            initString(&c);
            intToStringE(a->data.i, &c);
            x = &c;
            break;

        case VT_Double:
            initString(&c);
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
            initString(&d);
            intToStringE(b->data.i, &d);
            y = &d;
            break;

        case VT_Double:
            initString(&d);
            doubleToStringE(b->data.d, &d);
            y = &d;
            break;

        case VT_String:
            y = &(ret->data.s);
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

    if (!getError()) {
        ret->type = VT_Integer;
        ret->data.i = stringSubstrSearch(x, y);
    }

    if (x == &c)
        deleteString(&c);

    if (y == &d)
        deleteString(&d);
}

void getString(Value *ret)
{
    int c = getchar();
    char buffer[READ_BUFFER_SIZE];
    String *workingStr = &(ret->data.s);
    initString(workingStr);
    int idx;

    // Buffered read until EOL or EOF
    int i = 0;
    for (; c != '\n' || c != EOF; i++) {
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
                stringInitS(&str, "1", 1);
                workingStr = &str;
                len = 1;
                return;
            }
            else
                len = 0;
            break;

        case VT_Integer: {
            initString(&str);
            len = intToStringE(val->data.i, &str);
            workingStr = &str;
            break;
        }

        case VT_Double: {
            initString(&str);
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
    switch (val->type) {
        case VT_Null:
            ret->data.i = 0;
            break;

        case VT_Bool:
            ret->data.i = (int32_t)val->data.b;
            break;

        case VT_Integer:
            ret->data.i = val->data.i;
            break;

        case VT_Double:
            ret->data.i = (int32_t)val->data.d;
            break;

        case VT_String:
            ret->data.i = stringToInt(&(val->data.s));
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

void putString(Value *ret, Value *firstVal, int count)
{
    int i = 0;
    for (;i < count; i++, firstVal++){
        switch (firstVal->type) {
            case VT_Null:
                break;

            case VT_Bool:
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

void sortString(Value *val)
{
    switch (val->type) {
        case VT_Null:
            initString(&(val->data.s));
            break;

        case VT_Bool:
            if (val->data.b)
                initStringS(&(val->data.s), "1", 1);
            else
                initString(&(val->data.s));
            break;

        case VT_Integer:
            int i = val->data.i;
            initValue(val);
            initString(&(val->data.s));
            intToStringE(i, &(val->data.s));
            stringCharSort(&(val->data.s));
            break;

        case VT_Double:
            double d = val->data.d;
            initValue(val);
            initString(&(val->data.s));
            doubleToStringE(d, &(val->data.s));
            stringCharSort(&(val->data.s));
            break;

        case VT_String:
            stringCharSort(&(val->data.s));
            break;

        case VT_Undefined:
            setError(ERR_UndefVariable);
            return;

        case VT_StackPtr:
        case VT_InstructionPtr:
            setError(ERR_Internal);
            return;
    }
    val->type = VT_String;
}

int strLen(const Value *val)
{
    switch (val->type) {
        case VT_Null:
            return 0;

        case VT_Bool:
            // True => "1", False => ""
            return val->data.b ? 1 : 0;

        case VT_Integer:
            return intToStringE(val->data.i, NULL);

        case VT_Double:
            return doubleToStringE(val->data.d, NULL);

        case VT_String:
            return stringLength(&(val->data.s));

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

void strval(const Value *val, Value *ret)
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

        case VT_Integer:
            initString(&(ret->data.s));
            intToStringE(val->data.i, &(ret->data.s));
            break;

        case VT_Double:
            initString(&(ret->data.s));
            doubleToStringE(val->data.d, &(ret->data.s));
            break;

        case VT_String:
            initString(&(ret->data.s));
            stringCopy(&(val->data.s), &(ret->data.s));
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
