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
                aVal = vectorAt(constTable, instructionPtr->a);
                copyValue(aVal, resVal);
                break;

            case IST_Jmp:
                instructionPtr += instructionPtr->a;
                continue;

            case IST_Jmpz:
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);

                if (valueToBool(bVal))
                    instructionPtr += instructionPtr->a;
                else
                    instructionPtr++;

                // Clear space hold by condition's expression
                vectorResizeValue(stack, stackPtr + instructionPtr->res);

                continue;

            case IST_Push:
                vectorPushValue(stack, vectorAt(stack, instructionPtr->a));
                break;

            case IST_PushC:
                vectorPushValue(stack, vectorAtConst(constTable, instructionPtr->a));
                break;

            case IST_Reserve:
                aVal = vectorAt(stack,instructionPtr->a);
                for (int32_t i = 0; i < instructionPtr->a; i++)
                    vectorPushDefaultValue(stack);
                break;

            case IST_Pop:
                if (instructionPtr->a == 1)
                    vectorPopValue(stack);
                else {
                    // TODO
                }
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
                val.data.ip = instructionPtr;
                vectorPushValue(stack, &val);

                deleteValue(&val);

                instructionPtr = vectorAtConst(addressTable, instructionPtr->a);
                continue;
            }

            case IST_Return:
                // Load old instruction pointer
                aVal = vectorAt(stack, stackPtr + 1);
                if (aVal->data.ip != NULL) {
                    instructionPtr = aVal->data.ip;

                }
                else {
                    // End interpretation
                    running = 0;
                }

                // Load old stack pointer
                aVal = vectorAt(stack, stackPtr);
                stackPtr = aVal->data.sp;

                vectorResizeValue(stack, stackPtr - instructionPtr->a);
                continue;

            case IST_Nullify:
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                deleteValue(aVal);
                initValue(aVal);
                aVal->type = VT_Null;
                break;

            case IST_BoolVal:
                aVal = vectorAt(stack, stackPtr - 1);
                resVal = vectorAt(stack, stackPtr - 2);
                boolval(aVal, resVal);
                // Clear parameters
                vectorPopValue(stack);
                break;

            case IST_DoubleVal:
                aVal = vectorAt(stack, stackPtr - 1);
                resVal = vectorAt(stack, stackPtr - 2);
                doubleval(aVal, resVal);
                // Clear parameters
                vectorPopValue(stack);
                break;

            case IST_FindString:
                bVal = vectorAt(stack, stackPtr - 1);
                aVal = vectorAt(stack, stackPtr - 2);
                resVal = vectorAt(stack, stackPtr - 3);
                findString(resVal, aVal, bVal);
                // Clear parameters
                vectorPopNValue(stack, 2);
                break;

            case IST_GetString:
                resVal = vectorAt(stack, stackPtr - 1);
                getString(resVal);
                break;

            case IST_GetSubstring: {
                aVal = vectorAt(stack, stackPtr - 1);
                int end = valueToInt(aVal);

                aVal = vectorAt(stack, stackPtr - 2);
                int start = valueToInt(aVal);

                if (getError)
                    return;

                aVal = vectorAt(stack, stackPtr - 3);
                resVal = vectorAt(stack, stackPtr - 4);

                getSubstring(aVal, resVal, start, end);
                // Clear parameters
                vectorPopNValue(stack, 3);
                break;
            }

            case IST_IntVal:
                aVal = vectorAt(stack, stackPtr - 1);
                resVal = vectorAt(stack, stackPtr - 2);
                intval(aVal, resVal);
                // Clear parameters
                vectorPopValue(stack);
                break;

            case IST_PutString:
                aVal = vectorAt(stack, stackPtr - instructionPtr->a);
                resVal = vectorAt(stack, stackPtr - instructionPtr->a - 1);
                putString(resVal, aVal, instructionPtr->a);
                // Clear parameters
                vectorPopNValue(stack, instructionPtr->a);
                break;

            case IST_SortString:
                aVal = vectorAt(stack, stackPtr - 1);
                resVal = vectorAt(stack, stackPtr - 2);
                sortString(aVal, resVal);
                // Clear parameters
                vectorPopValue(stack);
                break;

            case IST_StrLen:
                aVal = vectorAt(stack, stackPtr - 1);
                resVal = vectorAt(stack, stackPtr - 2);
                strLen(aVal, resVal);
                // Clear parameters
                vectorPopValue(stack);
                break;

            case IST_StrVal:
                aVal = vectorAt(stack, stackPtr - 1);
                resVal = vectorAt(stack, stackPtr - 2);
                strval(aVal, resVal);
                // Clear parameters
                vectorPopValue(stack);
                break;
            case IST_Add:
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);
                resVal = vectorAt(stack, stackPtr + instructionPtr->res);
                if ((aVal->type == VT_Integer) && (bVal->type == VT_Integer)) {
                   resVal->data.i = aVal->data.i + bVal->data.i;
                   resVal->type = VT_Integer;
                }
                else if (aVal->type == VT_Double || bVal->type == VT_Integer) {
                   resVal->data.d = aVal->data.d + bVal->data.i;
                   resVal->type = VT_Double;
                }
                else if (aVal->type == VT_Integer || bVal->type == VT_Double) {
                   resVal->data.d = aVal->data.i + bVal->data.d;
                   resVal->type = VT_Double;
                }
                else if (aVal->type == VT_Double || bVal->type == VT_Double) {
                   resVal->data.d = aVal->data.d + bVal->data.d;
                   resVal->type = VT_Double;
                }

                break;

            case IST_Subtract:
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);
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
                break;

            case IST_Multiply:
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);
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
                break;

            case IST_Divide:
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);
                resVal = vectorAt(stack, stackPtr + instructionPtr->res);
                if (aVal->type == VT_Double && bVal->type == VT_Integer) {
                   resVal->data.d = aVal->data.d / bVal->data.i;
                   resVal->type = VT_Double;
                }
                else if (aVal->type == VT_Integer && bVal->type == VT_Double) {
                   resVal->data.d = aVal->data.i / bVal->data.d;
                   resVal->type = VT_Double;
                }
                else if (aVal->type == VT_Double && bVal->type == VT_Double) {
                   resVal->data.d = aVal->data.d / bVal->data.d;
                   resVal->type = VT_Double;
                }
                break;

            case IST_Concat:
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);
                resVal = vectorAt(stack, stackPtr + instructionPtr->res);
                strval(aVal,bVal);
                copyValue(aVal, resVal);
                break;

            case IST_Equal:
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);
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
                resVal = vectorAt(stack, stackPtr + instructionPtr->res);
                if (aVal->type == bVal->type ) {
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
                            if (stringCompare(&aVal->data.s,&bVal->data.s) != 0)
                                resVal->data.b = 1;
                            else
                                resVal->data.b = 0;
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
                    resVal->data.b = 0;
                }

                break;

            case IST_Less:
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);
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
                else {
                    resVal->type = VT_Bool;
                    resVal->data.b = 0;
                }

                break;


       case IST_LessEq:
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);
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
                else {
                    resVal->type = VT_Bool;
                    resVal->data.b = 0;
                }

                break;


        case IST_Greater:
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);
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
                else {
                    resVal->type = VT_Bool;
                    resVal->data.b = 0;
                }

                break;


        case IST_GreaterEq:
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);
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
                else {
                    resVal->type = VT_Bool;
                    resVal->data.b = 0;
                }

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

    // Create global stack frame
    {
        // Reserve space for return value
        vectorPushDefaultValue(stack);
        // Null old stack pointer
        vectorPushDefaultValue(stack);
        Value *tmp = vectorBack(stack);
        tmp->type = VT_InstructionPtr;
        tmp->data.ip = NULL;
        // Null old instruction pointer
        vectorPushDefaultValue(stack);
        tmp = vectorBack(stack);
        tmp->type = VT_StackPtr;
        tmp->data.sp = 0;
    }

    interpretationLoop(firstInstruction, constTable, addressTable, stack);

    freeValueVector(&stack);
}
