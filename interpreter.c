#include "interpreter.h"
#include "value_vector.h"
#include "builtin.h"
#include "nierr.h"

#include <stdint.h>

void interpretationLoop(const Instruction *firstInstruction, const Vector *constTable, const Vector *addressTable, Vector *stack)
{
    // 3 value pointers used during interpretation
    Value *resVal, *aVal, *bVal;

    // Stack pointer have to be index because of possibility of stack
    // vector reallocation
    uint32_t stackPtr = 1;

    const Instruction *instructionPtr = firstInstruction;
    uint8_t running = 1;

    while (running) {
        switch (instructionPtr->code) {
            case IST_Mov:
                resVal = vectorAt(stack, stackPtr + instructionPtr->res);
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                deleteValue(resVal);
                copyValue(aVal, resVal);
                break;

            case IST_MovC:
                resVal = vectorAt(stack, stackPtr + instructionPtr->res);
                deleteValue(resVal);
                copyValue(vectorAtConst(constTable, instructionPtr->a), resVal);
                break;

            case IST_Jmp:
                instructionPtr += instructionPtr->a;
                continue;

            case IST_Jmpz: {
                uint32_t tempRes = instructionPtr->res;
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);
                if (valueToBool(bVal))
                    instructionPtr++;
                else
                    instructionPtr += instructionPtr->a;

                // Clear space hold by condition's expression
                vectorResizeValue(stack, stackPtr + tempRes);

                continue;
            }

            case IST_Push:
                vectorPushValue(stack, vectorAt(stack, stackPtr + instructionPtr->a));
                break;

            case IST_PushC:
                vectorPushValue(stack, vectorAtConst(constTable, instructionPtr->a));
                break;

            case IST_Reserve:
                for (int32_t i = 0; i < instructionPtr->a; i++)
                    vectorPushDefaultValue(stack);
                break;

            case IST_Pop:
                if (instructionPtr->a == 1)
                    vectorPopValue(stack);
                else
                    vectorPopNValue(stack,instructionPtr->a);
                break;

            case IST_ClearExpr:
                vectorResizeValue(stack, stackPtr + instructionPtr->a);
                break;

            case IST_Call: {
                // Save stack and instruction pointers
                Value val;
                initValue(&val);

                // Stack pointer
                val.type = VT_StackPtr;
                val.data.sp = stackPtr;
                stackPtr = vectorSize(stack);
                vectorPushValue(stack, &val);

                // Instruction pointer
                val.type = VT_InstructionPtr;
                val.data.ip = instructionPtr + 1;
                vectorPushValue(stack, &val);

                deleteValue(&val);

                instructionPtr = *((Instruction**)vectorAtConst(addressTable, instructionPtr->a));
                continue;
            }

            case IST_Return:
                // Load old instruction pointer
                aVal = vectorAt(stack, stackPtr + 1);
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
                aVal = vectorAt(stack, stackPtr);
                stackPtr = aVal->data.sp;

                vectorResizeValue(stack, oldStackPtr - paramCount);
                continue;

            case IST_Nullify:
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                deleteValue(aVal);
                initValue(aVal);
                aVal->type = VT_Null;
                break;

            case IST_BoolVal: {
                uint32_t simStackPtr = vectorSize(stack);
                aVal = vectorAt(stack, simStackPtr - 1);
                resVal = vectorAt(stack, simStackPtr - 2);
                boolval(aVal, resVal);
                // Clear parameters
                vectorPopValue(stack);
                break;
            }

            case IST_DoubleVal: {
                uint32_t simStackPtr = vectorSize(stack);
                aVal = vectorAt(stack, simStackPtr - 1);
                resVal = vectorAt(stack, simStackPtr - 2);
                doubleval(aVal, resVal);
                // Clear parameters
                vectorPopValue(stack);
                break;
            }

            case IST_FindString: {
                uint32_t simStackPtr = vectorSize(stack);
                bVal = vectorAt(stack, simStackPtr - 1);
                aVal = vectorAt(stack, simStackPtr - 2);
                resVal = vectorAt(stack, simStackPtr - 3);
                findString(resVal, aVal, bVal);
                // Clear parameters
                vectorPopNValue(stack, 2);
                break;
            }

            case IST_GetString: {
                uint32_t simStackPtr = vectorSize(stack);
                resVal = vectorAt(stack, simStackPtr - 1);
                getString(resVal);
                break;
            }

            case IST_GetSubstring: {
                uint32_t simStackPtr = vectorSize(stack);
                aVal = vectorAt(stack, simStackPtr - 1);
                int end = valueToInt(aVal);

                aVal = vectorAt(stack, simStackPtr - 2);
                int start = valueToInt(aVal);

                if (getError())
                    return;

                aVal = vectorAt(stack, simStackPtr - 3);
                resVal = vectorAt(stack, simStackPtr - 4);

                getSubstring(aVal, resVal, start, end);
                // Clear parameters
                vectorPopNValue(stack, 3);
                break;
            }

            case IST_IntVal: {
                uint32_t simStackPtr = vectorSize(stack);
                aVal = vectorAt(stack, simStackPtr - 1);
                resVal = vectorAt(stack, simStackPtr - 2);
                intval(aVal, resVal);
                // Clear parameters
                vectorPopValue(stack);
                break;
            }

            case IST_PutString: {
                uint32_t simStackPtr = vectorSize(stack);
                aVal = vectorAt(stack, simStackPtr - instructionPtr->a);
                resVal = vectorAt(stack, simStackPtr - instructionPtr->a - 1);
                putString(resVal, aVal, instructionPtr->a);
                // Clear parameters
                vectorPopNValue(stack, instructionPtr->a);
                break;
            }

            case IST_SortString: {
                uint32_t simStackPtr = vectorSize(stack);
                aVal = vectorAt(stack, simStackPtr - 1);
                resVal = vectorAt(stack, simStackPtr - 2);
                sortString(aVal, resVal);
                // Clear parameters
                vectorPopValue(stack);
                break;
            }

            case IST_StrLen: {
                uint32_t simStackPtr = vectorSize(stack);
                aVal = vectorAt(stack, simStackPtr - 1);
                resVal = vectorAt(stack, simStackPtr - 2);
                strLen(aVal, resVal);
                // Clear parameters
                vectorPopValue(stack);
                break;
            }

            case IST_StrVal: {
                uint32_t simStackPtr = vectorSize(stack);
                aVal = vectorAt(stack, simStackPtr - 1);
                resVal = vectorAt(stack, simStackPtr - 2);
                strval(aVal, resVal);
                // Clear parameters
                vectorPopValue(stack);
                break;
            }

            case IST_Add:
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);
                vectorPushDefaultValue(stack);
                resVal = vectorAt(stack, stackPtr + instructionPtr->res);
                if ((aVal->type == VT_Integer) && (bVal->type == VT_Integer)) {
                   resVal->data.i = aVal->data.i + bVal->data.i;
                   resVal->type = VT_Integer;
                }
                else if (aVal->type == VT_Double && bVal->type == VT_Integer) {
                   resVal->data.d = aVal->data.d + bVal->data.i;
                   resVal->type = VT_Double;
                }
                else if (aVal->type == VT_Integer && bVal->type == VT_Double) {
                   resVal->data.d = aVal->data.i + bVal->data.d;
                   resVal->type = VT_Double;
                }
                else if (aVal->type == VT_Double && bVal->type == VT_Double) {
                   resVal->data.d = aVal->data.d + bVal->data.d;
                   resVal->type = VT_Double;
                }
                else
                    setError(ERR_OperandTypes);
                break;

            case IST_Subtract:
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);
                vectorPushDefaultValue(stack);
                resVal = vectorAt(stack, stackPtr + instructionPtr->res);
                if ((aVal->type == VT_Integer) && (bVal->type == VT_Integer)) {
                   resVal->data.i = aVal->data.i - bVal->data.i;
                   resVal->type = VT_Integer;
                }
                else if (aVal->type == VT_Double && bVal->type == VT_Integer) {
                   resVal->data.d = aVal->data.d - bVal->data.i;
                   resVal->type = VT_Double;
                }
                else if (aVal->type == VT_Integer && bVal->type == VT_Double) {
                   resVal->data.d = aVal->data.i - bVal->data.d;
                   resVal->type = VT_Double;
                }
                else if (aVal->type == VT_Double && bVal->type == VT_Double) {
                   resVal->data.d = aVal->data.d - bVal->data.d;
                   resVal->type = VT_Double;
                }
                else
                    setError(ERR_OperandTypes);
                break;

            case IST_Multiply:
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);
                vectorPushDefaultValue(stack);
                resVal = vectorAt(stack, stackPtr + instructionPtr->res);
                if ((aVal->type == VT_Integer) && (bVal->type == VT_Integer)) {
                   resVal->data.i = aVal->data.i * bVal->data.i;
                   resVal->type = VT_Integer;
                }
                else if (aVal->type == VT_Double && bVal->type == VT_Integer) {
                   resVal->data.d = aVal->data.d * bVal->data.i;
                   resVal->type = VT_Double;
                }
                else if (aVal->type == VT_Integer && bVal->type == VT_Double) {
                   resVal->data.d = aVal->data.i * bVal->data.d;
                   resVal->type = VT_Double;
                }
                else if (aVal->type == VT_Double && bVal->type == VT_Double) {
                   resVal->data.d = aVal->data.d * bVal->data.d;
                   resVal->type = VT_Double;
                }
                else
                    setError(ERR_OperandTypes);
                break;

            case IST_Divide:
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);
                vectorPushDefaultValue(stack);
                resVal = vectorAt(stack, stackPtr + instructionPtr->res);
                if (aVal->type == VT_Double && bVal->type == VT_Integer) {
                   if (bVal->data.i != 0) { 
                       resVal->data.d = aVal->data.d / (double)bVal->data.i;
                       resVal->type = VT_Double;
                   }
                   else
                       setError(ERR_DivideByZero);
                }
                else if (aVal->type == VT_Integer && bVal->type == VT_Double) {
                   if (bVal->data.d != 0) {
                       resVal->data.d = (double)aVal->data.i / bVal->data.d;
                       resVal->type = VT_Double;
                   }
                   else
                       setError(ERR_DivideByZero);
                }
                else if (aVal->type == VT_Double && bVal->type == VT_Double) {
                   if (bVal->data.d != 0) {
                       resVal->data.d = aVal->data.d / bVal->data.d;
                       resVal->type = VT_Double;
                   }
                   else
                       setError(ERR_DivideByZero);
                }
                else if (aVal->type == VT_Integer && bVal->type == VT_Integer) {
                   if (bVal->data.i != 0) {
                       resVal->data.i = (double)aVal->data.i / (double)bVal->data.i;
                       resVal->type = VT_Double;
                   }
                   else
                       setError(ERR_DivideByZero);
                }
                else
                    setError(ERR_OperandTypes);
                break;

            case IST_Concat:
                // Test if first operand is string
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                if (aVal->type != VT_String) {
                    setError(ERR_OperandTypes);
                    break;
                }

                // Copy aVal to resVal
                vectorPushDefaultValue(stack);
                resVal = vectorAt(stack, stackPtr + instructionPtr->res);
                initStringSet(&(resVal->data.s), &(aVal->data.s));

                // Add bVal, thus concatenating
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);
                if (bVal->type == VT_String)
                    stringAdd(&(resVal->data.s), &(bVal->data.s));
                else {
                    String tmp;
                    valueToString(bVal, &tmp);
                    if (getError())
                        return;

                    stringAdd(&(resVal->data.s), &tmp);
                    deleteString(&tmp);
                }

                resVal->type = VT_String;
                break;

            case IST_Equal:
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);
                vectorPushDefaultValue(stack);
                resVal = vectorAt(stack, stackPtr + instructionPtr->res);
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

                        case VT_String:
                            if (stringCompare(&aVal->data.s,&bVal->data.s) == 0)
                                resVal->data.b = 1;
                            else
                                resVal->data.b = 0;
                            resVal->type = VT_Bool;

                            break;

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
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);
                vectorPushDefaultValue(stack);
                resVal = vectorAt(stack, stackPtr + instructionPtr->res);
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

                        case VT_String:
                            resVal->data.b = stringCompare(&(aVal->data.s), &(bVal->data.s)) != 0;
                            resVal->type = VT_Bool;
                            break;

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
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);
                vectorPushDefaultValue(stack);
                resVal = vectorAt(stack, stackPtr + instructionPtr->res);
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

                        case VT_String:
                            if (stringCompare(&aVal->data.s,&bVal->data.s) == -1)
                                resVal->data.b = 1;
                            else
                                resVal->data.b = 0;
                            resVal->type = VT_Bool;

                            break;

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
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);
                vectorPushDefaultValue(stack);
                resVal = vectorAt(stack, stackPtr + instructionPtr->res);
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

                        case VT_String:
                            if (stringCompare(&aVal->data.s,&bVal->data.s) <= 0)
                                resVal->data.b = 1;
                            else
                                resVal->data.b = 0;
                            resVal->type = VT_Bool;

                            break;

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
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);
                vectorPushDefaultValue(stack);
                resVal = vectorAt(stack, stackPtr + instructionPtr->res);
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

                        case VT_String:
                            if (stringCompare(&aVal->data.s,&bVal->data.s) == 1)
                                resVal->data.b = 1;
                            else
                                resVal->data.b = 0;
                            resVal->type = VT_Bool;

                            break;

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
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);
                vectorPushDefaultValue(stack);
                resVal = vectorAt(stack, stackPtr + instructionPtr->res);
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

                        case VT_String:
                            if (stringCompare(&aVal->data.s,&bVal->data.s) >= 0)
                                resVal->data.b = 1;
                            else
                                resVal->data.b = 0;
                            resVal->type = VT_Bool;

                            break;

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

    // Reserve space for global return value
    vectorPushDefaultValue(stack);

    interpretationLoop(firstInstruction, constTable, addressTable, stack);

    freeValueVector(&stack);
}
