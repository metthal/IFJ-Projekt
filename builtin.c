#include "builtin.h"
#include "convert.h"
#include "nierr.h"
#include "ial.h"

#include <stdio.h>

#define READ_BUFFER_SIZE 64

BuiltinCode getBuiltinCode(const String *str)
{
    switch(str->data[0]) {
        case 'b':
            if (stringCompareS(str, "boolval", 7) == 0)
                return BTI_BoolVal;
            break;
        case 'd':
            if (stringCompareS(str, "doubleval", 9) == 0)
                return BTI_DoubleVal;
            break;
        case 'f':
            if (stringCompareS(str, "find_string", 11) == 0)
                return BTI_FindString;
            break;
        case 'g':
            if (stringCompareS(str, "get_string", 10) == 0)
                return BTI_GetString;
            else if (stringCompareS(str, "get_substring", 13) == 0)
                return BTI_GetSubstring;
            break;
        case 'i':
            if (stringCompareS(str, "intval", 6) == 0)
                return BTI_IntVal;
            break;
        case 'p':
            if (stringCompareS(str, "put_string", 10) == 0)
                return BTI_PutString;
            break;
        case 's':
            if (stringCompareS(str, "sort_string", 11) == 0)
                return BTI_SortString;
            else if (stringCompareS(str, "strlen", 6) == 0)
                return BTI_StrLen;
            else if (stringCompareS(str, "strval", 6) == 0)
                return BTI_StrVal;
            break;

        default:
            break;
    }

    return BTI_None;
}

int64_t getBuiltinParamCount(BuiltinCode code)
{
    switch (code) {
        case BTI_None:
            setError(ERR_Internal);
            return 0;
        case BTI_BoolVal:
            return 1;
        case BTI_DoubleVal:
            return 1;
        case BTI_FindString:
            return 2;
        case BTI_GetString:
            return 0;
        case BTI_GetSubstring:
            return 3;
        case BTI_IntVal:
            return 1;
        case BTI_PutString:
            return -1;
        case BTI_SortString:
            return 1;
        case BTI_StrLen:
            return 1;
        case BTI_StrVal:
            return 1;
    }
    return 0;
}

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

        default:
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

        default:
            setError(ERR_Internal);
            return 0;
    }
    return 0;
}

double valueToDouble(const Value *val)
{
    switch (val->type) {
        case VT_Null:
            return 0.0;
            break;

        case VT_Bool:
            return (double)val->data.b;
            break;

        case VT_Integer:
            return (double)val->data.i;
            break;

        case VT_Double:
            return val->data.d;
            break;

        case VT_String:
            return stringToDouble(&(val->data.s));
            break;

        case VT_Undefined:
            setError(ERR_UndefVariable);
            return 0.0;

        default:
            setError(ERR_Internal);
            return 0.0;
    }
    return 0.0;
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

        default:
            setError(ERR_Internal);
            return;
    }
}

void boolval(const Value *val, Value *ret)
{
    uint8_t tmp = valueToBool(val);
    deleteValue(ret);
    ret->data.b = tmp;
    ret->type = VT_Bool;
}

void doubleval(const Value *val, Value *ret)
{
    double tmp = valueToDouble(val);
    deleteValue(ret);
    ret->data.d = tmp;
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

        default:
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

        default:
            setError(ERR_Internal);
            // break because cleanup is required
            break;
    }

    if (getError()) {
        if (x == &c)
            deleteString(&c);
        return;
    }

    int tmp = stringSubstrSearch(x, y);
    deleteValue(ret);

    ret->data.i = tmp;
    ret->type = VT_Integer;

    if (x == &c)
        deleteString(&c);

    if (y == &d)
        deleteString(&d);
}

void getString(Value *ret)
{
    deleteValue(ret);

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
            if (getError())
                return;
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
    // Need to check so we won't delete our original value
    if (ret != val) {
        deleteValue(ret);
    }

    String *workingStr = &(ret->data.s);

    switch (val->type) {
        case VT_Null:
            len = 0;
            break;

        case VT_Bool:
            if (val->data.b) {
                initStringS(workingStr, "1", 1);
                len = 1;
            }
            else
                len = 0;
            break;

        case VT_Integer: {
            len = intToStringE(val->data.i, workingStr);
            break;
        }

        case VT_Double: {
            len = doubleToStringE(val->data.d, workingStr);
            break;
        }

        case VT_String:
            if (ret != val) {
                initStringSet(workingStr, &(val->data.s));
            }
            len = stringLength(workingStr);
            break;

        case VT_Undefined:
            setError(ERR_UndefVariable);
            return;

        default:
            setError(ERR_Internal);
            return;
    }

    if (start >= len || end > len) {
        setError(ERR_Substr);
        return;
    }

    if (!getError()) {
        stringSubstrI(workingStr, start, end - start);
        ret->type = VT_String;
    }
}

void intval(const Value *val, Value *ret)
{
    int tmp = valueToInt(val);
    deleteValue(ret);
    ret->data.i = tmp;
    ret->type = VT_Integer;
}

void putString(const Value *base, const Value *constBase, Value *ret, int count)
{
    const Value *it = base - count;
    const Value *item = it;
    while (it != base){
        switch (item->type) {
            case VT_Null:
                break;

            case VT_Bool:
                if (item->data.b)
                    putchar('1');
                break;

            case VT_Integer:
                printf("%d", item->data.i);
                break;

            case VT_Double:
                printf("%g", item->data.d);
                break;

            case VT_String:
                printf("%s", item->data.s.data);
                break;

            case VT_StrongReference:
            case VT_WeakReference:
                item += it->data.ref;
                continue;

            case VT_ConstReference:
                item = constBase + it->data.ref;
                continue;

            case VT_Undefined:
                setError(ERR_UndefVariable);
                return;

            default:
                setError(ERR_Internal);
                return;
        }

        it++;
        item = it;
    }

    deleteValue(ret);
    ret->data.i = count;
    ret->type = VT_Integer;
}

void sortString(const Value *val, Value *ret)
{
    String *workingStr = &(ret->data.s);

    // Need to check so we won't delete our original value
    if (val != ret) {
        deleteValue(ret);
    }

    switch (val->type) {
        case VT_Null:
            initString(workingStr);
            break;

        case VT_Bool:
            if (val->data.b)
                initStringS(workingStr, "1", 1);
            else
                initString(workingStr);
            break;

        case VT_Integer: {
            intToStringE(val->data.i, workingStr);
            stringCharSort(workingStr);
            break;
        }

        case VT_Double: {
            doubleToStringE(val->data.d, workingStr);
            stringCharSort(workingStr);
            break;
        }

        case VT_String:
            // We need to check so we won't accidentaly delete val->data.s string
            if (val != ret) {
                initStringSet(workingStr, &(val->data.s));
            }
            stringCharSort(workingStr);
            break;

        case VT_Undefined:
            setError(ERR_UndefVariable);
            return;

        default:
            setError(ERR_Internal);
            return;
    }

    ret->type = VT_String;
}

void strLen(const Value *val, Value *ret)
{
    int tmp;
    switch (val->type) {
        case VT_Null:
            tmp = 0;
            break;

        case VT_Bool:
            // True => "1", False => ""
            tmp = val->data.b ? 1 : 0;
            break;

        case VT_Integer:
            tmp = intToStringE(val->data.i, NULL);
            break;

        case VT_Double:
            tmp = doubleToStringE(val->data.d, NULL);
            break;

        case VT_String:
            tmp = stringLength(&(val->data.s));
            break;

        case VT_Undefined:
            setError(ERR_UndefVariable);
            return;

        default:
            setError(ERR_Internal);
            return;
    }

    deleteValue(ret);
    ret->data.i = tmp;
    ret->type = VT_Integer;
}

void strval(const Value *val, Value *ret)
{
    if (ret == val && val->type == VT_String)
        return;

    valueToString(val, &(ret->data.s));
    ret->type = VT_String;
}
