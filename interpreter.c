#include "interpreter.h"
#include "value_vector.h"
#include "builtin.h"
#include "nierr.h"

#include <stdint.h>

#define DEFAULT_STACK_CAPACITY 5000

void fillResValuePtr(Value *base, Value **res, int32_t ires)
{
    *res = base + ires;
    if ((*res)->type == VT_StrongReference)
        *res += (*res)->data.ref;
}

uint8_t fillConstValuePtr(Value *base, const Value *constBase, const Value **x,
        uint8_t mode, int32_t ix)
{
    //TODO DON'T YOU, FORGET ABOUT PUT_STRING!!!
    //TODO Don't, don't, don't, don't
    //TODO DON'T YOU, FORGET ABOUT PUT_STRING!!!
    //TODO http://www.youtube.com/watch?v=CdqoNKCCt7A

    *x = base + ix;
    switch ((*x)->type){
        case VT_StrongReference:
        case VT_WeakReference:
            *x += (*x)->data.ref;
            break;
        case VT_ConstReference:
            *x = constBase + (*x)->data.ref;
            break;
        case VT_Undefined:
            setError(ERR_UndefVariable);
            return 0;
        default:
            break;
    }

    return 1;
}

uint8_t fillResConstValuePtr(Value *base, const Value *constBase, Value **res, const Value **x,
        uint8_t mode, int32_t ires, int32_t ix)
{
    *res = base + ires;
    if ((*res)->type == VT_StrongReference)
        *res += (*res)->data.ref;

    *x = base + ix;
    switch ((*x)->type){
        case VT_StrongReference:
        case VT_WeakReference:
            *x += (*x)->data.ref;
            break;
        case VT_ConstReference:
            *x = constBase + (*x)->data.ref;
            break;
        case VT_Undefined:
            setError(ERR_UndefVariable);
            return 0;
        default:
            break;
    }

    return 1;
}

uint8_t fillConstValuePtrs(Value *base, const Value *constBase, const Value **a, const Value **b,
        uint8_t mode, int32_t ia, int32_t ib)
{
    *a = base + ia;
    switch ((*a)->type){
        case VT_StrongReference:
        case VT_WeakReference:
            *a += (*a)->data.ref;
            break;
        case VT_ConstReference:
            *a = constBase + (*a)->data.ref;
            break;
        case VT_Undefined:
            setError(ERR_UndefVariable);
            return 0;
        default:
            break;
    }

    *b = base + ib;
    switch ((*b)->type){
        case VT_StrongReference:
        case VT_WeakReference:
            *b += (*b)->data.ref;
            break;
        case VT_ConstReference:
            *b = constBase + (*b)->data.ref;
            break;
        case VT_Undefined:
            setError(ERR_UndefVariable);
            return 0;
        default:
            break;
    }

    return 1;
}

uint8_t fillValuePtrs(Value *base, const Value *constBase, Value **res, const Value **a, const Value **b,
        uint8_t mode, int32_t ires, int32_t ia, int32_t ib)
{
    *res = base + ires;
    if ((*res)->type == VT_StrongReference)
        *res += (*res)->data.ref;

    *a = base + ia;
    switch ((*a)->type){
        case VT_StrongReference:
        case VT_WeakReference:
            *a += (*a)->data.ref;
            break;
        case VT_ConstReference:
            *a = constBase + (*a)->data.ref;
            break;
        case VT_Undefined:
            setError(ERR_UndefVariable);
            return 0;
        default:
            break;
    }

    *b = base + ib;
    switch ((*b)->type){
        case VT_StrongReference:
        case VT_WeakReference:
            *b += (*b)->data.ref;
            break;
        case VT_ConstReference:
            *b = constBase + (*b)->data.ref;
            break;
        case VT_Undefined:
            setError(ERR_UndefVariable);
            return 0;
        default:
            break;
    }

    return 1;
}

void interpretationLoop(const Instruction *firstInstruction, const Vector *constTable, const Vector *addressTable, Vector *stack)
{
    // 3 value pointers used during interpretation
    Value *resVal;
    const Value *aVal, *bVal;

    // Stack pointer have to be index because of possibility of stack
    // vector reallocation
    uint32_t stackPtr = 1;

    // Pointer on first constant for fast access
    const Value *cCtIt = vectorAtConst(constTable, 0);

    const Instruction *instructionPtr = firstInstruction;
    uint8_t running = 1;

    while (running) {
        switch (instructionPtr->code) {
            case IST_Mov: {
                // Don't dereference, as we can just move
                // reference and update it.
                Value *base = vectorFastAtValue(stack, stackPtr);
                aVal = base + instructionPtr->a;

                // There's exception with WeakReference, which
                // must be dereferenced
                if (aVal->type == VT_WeakReference)
                    aVal += aVal->data.ref;

                fillResValuePtr(base, &resVal, instructionPtr->res);

                if (aVal != resVal) {
                    deleteValue(resVal);
                    copyValue(aVal, resVal);

                    // Reference needs to be updated
                    if (aVal->type == VT_StrongReference)
                        resVal->data.ref += (aVal - resVal);
                }
                break;
            }

            case IST_MovC:
                resVal = vectorFastAtValue(stack, stackPtr + instructionPtr->res);
                resVal->data.ref = instructionPtr->a;
                resVal->type = VT_ConstReference;
                break;

            case IST_Jmp:
                instructionPtr += instructionPtr->a;
                continue;

            case IST_Jmpz: {
                if (!fillConstValuePtr(vectorFastAtValue(stack, stackPtr), cCtIt, &bVal,
                        0, instructionPtr->b))
                    break;

                uint32_t tempRes = instructionPtr->res;
                if (valueToBool(bVal))
                    instructionPtr++;
                else
                    instructionPtr += instructionPtr->a;

                // If valueToBool failed
                if (getError())
                    return;

                // Clear space hold by condition's expression
                vectorDownsizeValue(stack, stackPtr + tempRes);

                continue;
            }

            case IST_Push: {
                if ((vectorFastAtValue(stack, stackPtr + instructionPtr->a))->type == VT_String) {
                    resVal = vectorPushDefaultValue(stack);
                    resVal->data.ref = ((int64_t)stackPtr + instructionPtr->a - (vectorSize(stack) - 1));
                    resVal->type = VT_WeakReference;
                }
                else {
                    // Reference gets updated automatically
                    resVal = vectorPushIndexValue(stack, stackPtr + instructionPtr->a);
                }

                break;
            }

            case IST_PushC:
                resVal = vectorPushDefaultValue(stack);
                resVal->data.ref = instructionPtr->a;
                resVal->type = VT_ConstReference;
                break;

            case IST_PushRef:
                resVal = vectorPushDefaultValue(stack);
                aVal = resVal + instructionPtr->a;
                switch(aVal->type) {
                    case VT_StrongReference:
                        resVal->data.ref = aVal->data.ref + instructionPtr->a;
                        resVal->type = VT_StrongReference;
                        break;

                    default:
                        resVal->data.ref = instructionPtr->a;
                        resVal->type = VT_StrongReference;
                        break;
                }
                break;

            case IST_Reserve:
                vectorPushDefaultNValue(stack, instructionPtr->a);
                break;

            case IST_Pop:
                if (instructionPtr->a == 1)
                    vectorPopValue(stack);
                else
                    vectorPopNValue(stack, instructionPtr->a);
                break;

            case IST_ClearExpr:
                vectorDownsizeValue(stack, stackPtr + instructionPtr->a);
                break;

            case IST_Call: {
                // Stack pointer
                Value *val = vectorPushDefaultValue(stack);
                val->type = VT_StackPtr;
                val->data.sp = stackPtr;
                stackPtr = vectorSize(stack) - 1;

                // Instruction pointer
                val = vectorPushDefaultValue(stack);
                val->type = VT_InstructionPtr;
                val->data.ip = instructionPtr + 1;

                instructionPtr = *((Instruction**)vectorAtConst(addressTable, instructionPtr->a));
                continue;
            }

            case IST_Return:
                // Load old instruction pointer
                aVal = vectorFastAtValue(stack, stackPtr + 1);
                uint32_t paramCount = instructionPtr->a;
                if (aVal->type == VT_Null) {
                    // End interpretation
                    running = 0;
                    return;
                }
                else
                    instructionPtr = aVal->data.ip;

                // Load old stack pointer
                uint32_t oldStackPtr = stackPtr;
                aVal = vectorFastAtValue(stack, stackPtr);
                stackPtr = aVal->data.sp;

                vectorDownsizeValue(stack, oldStackPtr - paramCount);
                continue;

            case IST_Nullify:
                fillResValuePtr(vectorFastAtValue(stack, stackPtr), &resVal, instructionPtr->a);

                deleteValue(resVal);
                initValue(resVal);
                resVal->type = VT_Null;
                break;

            case IST_BoolVal: {
                if (!fillResConstValuePtr(vectorEndValue(stack), cCtIt, &resVal, &aVal,
                        0, -2, -1))
                    break;

                boolval(aVal, resVal);
                // Clear parameters
                vectorPopValue(stack);
                break;
            }

            case IST_DoubleVal: {
                if (!fillResConstValuePtr(vectorEndValue(stack), cCtIt, &resVal, &aVal,
                        0, -2, -1))
                    break;

                doubleval(aVal, resVal);
                // Clear parameters
                vectorPopValue(stack);
                break;
            }

            case IST_FindString: {
                if (!fillValuePtrs(vectorEndValue(stack), cCtIt, &resVal, &aVal, &bVal,
                        0, -3, -2, -1))
                    break;

                findString(resVal, aVal, bVal);
                // Clear parameters
                vectorPopNValue(stack, 2);
                break;
            }

            case IST_GetString: {
                fillResValuePtr(vectorEndValue(stack), &resVal, -1);

                getString(resVal);
                break;
            }

            case IST_GetSubstring: {
                if (!fillConstValuePtrs(vectorEndValue(stack), cCtIt, &aVal, &bVal,
                        0, -2, -1))
                    break;

                int start = valueToInt(aVal);
                int end = valueToInt(bVal);

                if (getError())
                    return;

                if (!fillResConstValuePtr(vectorEndValue(stack), cCtIt, &resVal, &aVal,
                        0, -4, -3))
                    break;

                getSubstring(aVal, resVal, start, end);
                // Clear parameters
                vectorPopNValue(stack, 3);
                break;
            }

            case IST_IntVal: {
                if (!fillResConstValuePtr(vectorEndValue(stack), cCtIt, &resVal, &aVal,
                        0, -2, -1))
                    break;

                intval(aVal, resVal);
                // Clear parameters
                vectorPopValue(stack);
                break;
            }

            case IST_PutString: {
                Value *base = vectorEndValue(stack);
                fillResValuePtr(base, &resVal, -instructionPtr->a - 1);

                putString(base, cCtIt, resVal, instructionPtr->a);
                // Clear parameters
                vectorPopNValue(stack, instructionPtr->a);
                break;
            }

            case IST_SortString: {
                if (!fillResConstValuePtr(vectorEndValue(stack), cCtIt, &resVal, &aVal,
                        0, -2, -1))
                    break;

                sortString(aVal, resVal);
                // Clear parameters
                vectorPopValue(stack);
                break;
            }

            case IST_StrLen: {
                if (!fillResConstValuePtr(vectorEndValue(stack), cCtIt, &resVal, &aVal,
                        0, -2, -1))
                    break;

                strLen(aVal, resVal);
                // Clear parameters
                vectorPopValue(stack);
                break;
            }

            case IST_StrVal: {
                if (!fillResConstValuePtr(vectorEndValue(stack), cCtIt, &resVal, &aVal,
                        0, -2, -1))
                    break;

                strval(aVal, resVal);
                // Clear parameters
                vectorPopValue(stack);
                break;
            }

            case IST_Add:
                if (stackPtr + instructionPtr->res >= vectorSize(stack))
                    vectorPushDefaultValue(stack);
                if (!fillValuePtrs(vectorFastAtValue(stack, stackPtr), cCtIt, &resVal, &aVal, &bVal,
                        0, instructionPtr->res, instructionPtr->a, instructionPtr->b))
                    break;

                if (aVal->type == VT_Integer) {
                    if (bVal->type == VT_Integer) {
                        resVal->data.i = aVal->data.i + bVal->data.i;
                        resVal->type = VT_Integer;
                    }
                    else if (bVal->type == VT_Double) {
                        resVal->data.d = aVal->data.i + bVal->data.d;
                        resVal->type = VT_Double;
                    }
                    else
                        setError(ERR_OperandTypes);

                    break;
                }
                else if (aVal->type == VT_Double) {
                    if (bVal->type == VT_Integer) {
                        resVal->data.d = aVal->data.d + bVal->data.i;
                        resVal->type = VT_Double;
                    }
                    else if (bVal->type == VT_Double) {
                        resVal->data.d = aVal->data.d + bVal->data.d;
                        resVal->type = VT_Double;
                    }
                    else
                        setError(ERR_OperandTypes);

                    break;
                }
                else
                    setError(ERR_OperandTypes);

                break;

            case IST_Subtract:
                if (stackPtr + instructionPtr->res >= vectorSize(stack))
                    vectorPushDefaultValue(stack);
                if (!fillValuePtrs(vectorFastAtValue(stack, stackPtr), cCtIt, &resVal, &aVal, &bVal,
                        0, instructionPtr->res, instructionPtr->a, instructionPtr->b))
                    break;

                if (aVal->type == VT_Integer) {
                    if (bVal->type == VT_Integer) {
                        resVal->data.i = aVal->data.i - bVal->data.i;
                        resVal->type = VT_Integer;
                    }
                    else if (bVal->type == VT_Double) {
                        resVal->data.d = aVal->data.i - bVal->data.d;
                        resVal->type = VT_Double;
                    }
                    else
                        setError(ERR_OperandTypes);

                    break;
                }
                else if (aVal->type == VT_Double) {
                    if (bVal->type == VT_Integer) {
                        resVal->data.d = aVal->data.d - bVal->data.i;
                        resVal->type = VT_Double;
                    }
                    else if (bVal->type == VT_Double) {
                        resVal->data.d = aVal->data.d - bVal->data.d;
                        resVal->type = VT_Double;
                    }
                    else
                        setError(ERR_OperandTypes);

                    break;
                }
                else
                    setError(ERR_OperandTypes);

                break;

            case IST_Multiply:
                if (stackPtr + instructionPtr->res >= vectorSize(stack))
                    vectorPushDefaultValue(stack);
                if (!fillValuePtrs(vectorFastAtValue(stack, stackPtr), cCtIt, &resVal, &aVal, &bVal,
                        0, instructionPtr->res, instructionPtr->a, instructionPtr->b))
                    break;

                if (aVal->type == VT_Integer) {
                    if (bVal->type == VT_Integer) {
                        resVal->data.i = aVal->data.i * bVal->data.i;
                        resVal->type = VT_Integer;
                    }
                    else if (bVal->type == VT_Double) {
                        resVal->data.d = aVal->data.i * bVal->data.d;
                        resVal->type = VT_Double;
                    }
                    else
                        setError(ERR_OperandTypes);

                    break;
                }
                else if (aVal->type == VT_Double) {
                    if (bVal->type == VT_Integer) {
                        resVal->data.d = aVal->data.d * bVal->data.i;
                        resVal->type = VT_Double;
                    }
                    else if (bVal->type == VT_Double) {
                        resVal->data.d = aVal->data.d * bVal->data.d;
                        resVal->type = VT_Double;
                    }
                    else
                        setError(ERR_OperandTypes);

                    break;
                }
                else
                    setError(ERR_OperandTypes);

                break;

            case IST_Divide:
                if (stackPtr + instructionPtr->res >= vectorSize(stack))
                    vectorPushDefaultValue(stack);
                if (!fillValuePtrs(vectorFastAtValue(stack, stackPtr), cCtIt, &resVal, &aVal, &bVal,
                        0, instructionPtr->res, instructionPtr->a, instructionPtr->b))
                    break;

                if (bVal->type == VT_Integer) {
                    if (bVal->data.i == 0) {
                        setError(ERR_DivideByZero);
                        break;
                    }

                    if (aVal->type == VT_Integer) {
                        resVal->data.d = (double)aVal->data.i / (double)bVal->data.i;
                        resVal->type = VT_Double;
                    }
                    else if (aVal->type == VT_Double) {
                        resVal->data.d = aVal->data.d / (double)bVal->data.i;
                        resVal->type = VT_Double;
                    }
                    else
                        setError(ERR_OperandTypes);

                    break;
                }
                else if (bVal->type == VT_Double) {
                    if (bVal->data.d == 0.0) {
                        setError(ERR_DivideByZero);
                        break;
                    }

                    if (aVal->type == VT_Integer) {
                        resVal->data.d = (double)aVal->data.i / bVal->data.d;
                        resVal->type = VT_Double;

                    }
                    else if (aVal->type == VT_Double) {
                        resVal->data.d = aVal->data.d / bVal->data.d;
                        resVal->type = VT_Double;
                    }
                    else
                        setError(ERR_OperandTypes);

                    break;
                }
                else
                    setError(ERR_OperandTypes);

                break;

            case IST_Concat:
                if (stackPtr + instructionPtr->res >= vectorSize(stack))
                    vectorPushDefaultValue(stack);
                if (!fillValuePtrs(vectorFastAtValue(stack, stackPtr), cCtIt, &resVal, &aVal, &bVal,
                        0, instructionPtr->res, instructionPtr->a, instructionPtr->b))
                    break;

                // Test if first operand is string
                if (aVal->type != VT_String) {
                    setError(ERR_OperandTypes);
                    break;
                }

                // If resVal == aVal, we can just add bVal
                if (resVal != aVal) {
                    if (resVal == bVal && bVal->type == VT_String) {
                        // Special case, aVal can be just added before resVal == bVal
                        stringFrontAdd(&(resVal->data.s), &(aVal->data.s));
                        break;
                    }
                }

                // Add bVal, thus concatenating
                if (bVal->type == VT_String) {
                    // Initialize result if it's not already set to aVal
                    if (resVal != aVal) {
                        deleteValue(resVal);
                        initStringSet(&(resVal->data.s), &(aVal->data.s));
                    }

                    // This will work even if resVal == aVal == bVal
                    stringAdd(&(resVal->data.s), &(bVal->data.s));
                }
                else {
                    // First, bVal needs to be converted
                    String tmp;
                    valueToString(bVal, &tmp);
                    if (getError())
                        return;

                    // Then, having temporary string, we can initialize resVal
                    // but only if it's not aVal
                    if (resVal != aVal) {
                        deleteValue(resVal);
                        initStringSet(&(resVal->data.s), &(aVal->data.s));
                    }

                    // And perform concatenation
                    stringAdd(&(resVal->data.s), &tmp);

                    // Finally, delete temporary string
                    deleteString(&tmp);
                }

                resVal->type = VT_String;
                break;

            case IST_Equal:
                if (stackPtr + instructionPtr->res >= vectorSize(stack))
                    vectorPushDefaultValue(stack);
                if (!fillValuePtrs(vectorFastAtValue(stack, stackPtr), cCtIt, &resVal, &aVal, &bVal,
                        0, instructionPtr->res, instructionPtr->a, instructionPtr->b))
                    break;

                if (aVal->type == bVal->type ) {
                    switch (aVal->type) {
                        case VT_Integer:
                            resVal->data.b = aVal->data.i == bVal->data.i;
                            resVal->type = VT_Bool;
                            break;

                        case VT_Double:
                            resVal->data.b = aVal->data.d == bVal->data.d;
                            resVal->type = VT_Bool;
                            break;

                        case VT_String: {
                            uint8_t tmp = stringCompare(&aVal->data.s, &bVal->data.s) == 0;
                            deleteValue(resVal);
                            resVal->data.b = tmp;
                            resVal->type = VT_Bool;
                            break;
                        }

                        case VT_Bool:
                            resVal->data.b = aVal->data.b == bVal->data.b;
                            resVal->type = VT_Bool;

                            break;
                        default:
                            resVal->type = VT_Bool;
                            resVal->data.b = 0;
                            break;
                    }

                }
                else {
                    resVal->type = VT_Bool;
                    resVal->data.b = 0;
                }

                break;

            case IST_NotEqual:
                if (stackPtr + instructionPtr->res >= vectorSize(stack))
                    vectorPushDefaultValue(stack);
                if (!fillValuePtrs(vectorFastAtValue(stack, stackPtr), cCtIt, &resVal, &aVal, &bVal,
                        0, instructionPtr->res, instructionPtr->a, instructionPtr->b))
                    break;

                if (aVal->type == bVal->type) {
                    switch (aVal->type) {
                        case VT_Integer:
                            resVal->data.b = aVal->data.i != bVal->data.i;
                            resVal->type = VT_Bool;
                            break;

                        case VT_Double:
                            resVal->data.b = aVal->data.d != bVal->data.d;
                            resVal->type = VT_Bool;
                            break;

                        case VT_String: {
                            uint8_t tmp = stringCompare(&aVal->data.s, &bVal->data.s) != 0;
                            deleteValue(resVal);
                            resVal->data.b = tmp;
                            resVal->type = VT_Bool;
                            break;
                        }

                        case VT_Bool:
                            resVal->data.b = aVal->data.b != bVal->data.b;
                            resVal->type = VT_Bool;

                            break;
                        default:
                        resVal->type = VT_Bool;
                        resVal->data.b = 0;
                        break;
                    }

                }
                else {
                    resVal->type = VT_Bool;
                    resVal->data.b = 1;
                }

                break;

            case IST_Less:
                if (stackPtr + instructionPtr->res >= vectorSize(stack))
                    vectorPushDefaultValue(stack);
                if (!fillValuePtrs(vectorFastAtValue(stack, stackPtr), cCtIt, &resVal, &aVal, &bVal,
                        0, instructionPtr->res, instructionPtr->a, instructionPtr->b))
                    break;

                if (aVal->type == bVal->type ) {
                    switch (aVal->type) {
                        case VT_Integer:
                            resVal->data.b = aVal->data.i < bVal->data.i;
                            resVal->type = VT_Bool;
                            break;

                        case VT_Double:
                            resVal->data.b = aVal->data.d < bVal->data.d;
                            resVal->type = VT_Bool;
                            break;

                        case VT_String: {
                            uint8_t tmp = stringCompare(&aVal->data.s, &bVal->data.s) == -1;
                            deleteValue(resVal);
                            resVal->data.b = tmp;
                            resVal->type = VT_Bool;
                            break;
                        }

                        case VT_Bool:
                            resVal->data.b = aVal->data.b < bVal->data.b;
                            resVal->type = VT_Bool;

                            break;
                        default:
                        resVal->type = VT_Bool;
                        resVal->data.b = 0;
                        break;
                    }

                }
                else
                    setError(ERR_OperandTypes);

                break;


            case IST_LessEq:
                if (stackPtr + instructionPtr->res >= vectorSize(stack))
                    vectorPushDefaultValue(stack);
                if (!fillValuePtrs(vectorFastAtValue(stack, stackPtr), cCtIt, &resVal, &aVal, &bVal,
                        0, instructionPtr->res, instructionPtr->a, instructionPtr->b))
                    break;

                if (aVal->type == bVal->type ) {
                    switch (aVal->type) {
                        case VT_Integer:
                            resVal->data.b = aVal->data.i <= bVal->data.i;
                            resVal->type = VT_Bool;
                            break;

                        case VT_Double:
                            resVal->data.b = aVal->data.d <= bVal->data.d;
                            resVal->type = VT_Bool;
                            break;

                        case VT_String: {
                            uint8_t tmp = stringCompare(&aVal->data.s, &bVal->data.s) <= 0;
                            deleteValue(resVal);
                            resVal->data.b = tmp;
                            resVal->type = VT_Bool;
                            break;
                        }

                        case VT_Bool:
                            resVal->data.b = aVal->data.b <= bVal->data.b;
                            resVal->type = VT_Bool;

                            break;
                        default:
                        resVal->type = VT_Bool;
                        resVal->data.b = 0;
                        break;
                    }

                }
                else
                    setError(ERR_OperandTypes);

                break;


            case IST_Greater:
                if (stackPtr + instructionPtr->res >= vectorSize(stack))
                    vectorPushDefaultValue(stack);
                if (!fillValuePtrs(vectorFastAtValue(stack, stackPtr), cCtIt, &resVal, &aVal, &bVal,
                        0, instructionPtr->res, instructionPtr->a, instructionPtr->b))
                    break;

                if (aVal->type == bVal->type ) {
                    switch (aVal->type) {
                        case VT_Integer:
                            resVal->data.b = aVal->data.i > bVal->data.i;
                            resVal->type = VT_Bool;
                            break;

                        case VT_Double:
                            resVal->data.b = aVal->data.d > bVal->data.d;
                            resVal->type = VT_Bool;
                            break;

                        case VT_String: {
                            uint8_t tmp = stringCompare(&aVal->data.s, &bVal->data.s) == 1;
                            deleteValue(resVal);
                            resVal->data.b = tmp;
                            resVal->type = VT_Bool;
                            break;
                        }

                        case VT_Bool:
                            resVal->data.b = aVal->data.b > bVal->data.b;
                            resVal->type = VT_Bool;

                            break;
                        default:
                        resVal->type = VT_Bool;
                        resVal->data.b = 0;
                        break;
                    }

                }
                else
                    setError(ERR_OperandTypes);

                break;


            case IST_GreaterEq:
                if (stackPtr + instructionPtr->res >= vectorSize(stack))
                    vectorPushDefaultValue(stack);
                if (!fillValuePtrs(vectorFastAtValue(stack, stackPtr), cCtIt, &resVal, &aVal, &bVal,
                        0, instructionPtr->res, instructionPtr->a, instructionPtr->b))
                    break;

                if (aVal->type == bVal->type ) {
                    switch (aVal->type) {
                        case VT_Integer:
                            resVal->data.b = aVal->data.i >= bVal->data.i;
                            resVal->type = VT_Bool;
                            break;

                        case VT_Double:
                            resVal->data.b = aVal->data.d >= bVal->data.d;
                            resVal->type = VT_Bool;
                            break;

                        case VT_String: {
                            uint8_t tmp = stringCompare(&aVal->data.s, &bVal->data.s) >= 0;
                            deleteValue(resVal);
                            resVal->data.b = tmp;
                            resVal->type = VT_Bool;
                            break;
                        }

                        case VT_Bool:
                            resVal->data.b = aVal->data.b >= bVal->data.b;
                            resVal->type = VT_Bool;

                            break;

                        default:
                        resVal->type = VT_Bool;
                        resVal->data.b = 0;
                        break;
                    }

                }
                else
                    setError(ERR_OperandTypes);

                break;

            case IST_And: {
                if (stackPtr + instructionPtr->res >= vectorSize(stack))
                    vectorPushDefaultValue(stack);
                if (!fillValuePtrs(vectorFastAtValue(stack, stackPtr), cCtIt, &resVal, &aVal, &bVal,
                        0, instructionPtr->res, instructionPtr->a, instructionPtr->b))
                    break;

                uint8_t tmp = valueToBool(aVal) && valueToBool(bVal);
                deleteValue(resVal);
                resVal->data.b = tmp;
                resVal->type = VT_Bool;

                break;
            }

            case IST_Or: {
                if (stackPtr + instructionPtr->res >= vectorSize(stack))
                    vectorPushDefaultValue(stack);
                if (!fillValuePtrs(vectorFastAtValue(stack, stackPtr), cCtIt, &resVal, &aVal, &bVal,
                        0, instructionPtr->res, instructionPtr->a, instructionPtr->b))
                    break;

                uint8_t tmp = valueToBool(aVal) || valueToBool(bVal);
                deleteValue(resVal);
                resVal->data.b = tmp;
                resVal->type = VT_Bool;

                break;
            }

            case IST_Not: {
                if (stackPtr + instructionPtr->res >= vectorSize(stack))
                    vectorPushDefaultValue(stack);
                if (!fillResConstValuePtr(vectorFastAtValue(stack, stackPtr), cCtIt, &resVal, &aVal,
                        0, instructionPtr->res, instructionPtr->a))
                    break;

                // if B is 1 it means we are actually performing negation (used in case of odd number of negations)
                // if B is 0 it means we just convert value to bool (used in case of even number of negations)
                // A is TRUE(1) and B is TRUE(1) = 1 ^ 1 = FALSE(0)
                // A is FALSE(0) and B is TRUE(1) = 0 ^ 1 = TRUE(1)
                // A is TRUE(1) and B is FALSE(0) = 1 ^ 0 = TRUE(1)
                // A is FALSE(0) and B is FALSE(0) = 0 ^ 0 = FALSE(0)
                uint8_t tmp = valueToBool(aVal) ^ instructionPtr->b;
                deleteValue(resVal);
                resVal->data.b = tmp;
                resVal->type = VT_Bool;

                break;
            }

            default:
                break;
        }

        if(getError())
            return;

        // Move to next instruction
        instructionPtr++;
    }
}

void interpret(const Instruction *firstInstruction, const Vector *constTable, const Vector *addressTable)
{
    Vector *stack = newValueVector();
    vectorReserve(stack, DEFAULT_STACK_CAPACITY);

    // Reserve space for global return value
    vectorPushDefaultValue(stack);

    interpretationLoop(firstInstruction, constTable, addressTable, stack);

    freeValueVector(&stack);
}
