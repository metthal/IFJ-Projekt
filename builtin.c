#include "builtin.h"
#include "nierr.h"

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
            setError(ERR_Convert);
            return;
    }
    ret->type = VT_Bool;
}

void doubleval(const Value *val, Value *ret)
{

}

void findString()
{

}

void getString()
{

}

void getSubstring()
{

}

void intval(const Value *val, Value *ret)
{

}

void putString()
{

}

void sortString()
{

}

void strLen()
{

}

void strval(const Value *val, Value *ret)
{

}
