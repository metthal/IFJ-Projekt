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
                // TODO
                break;

            case IST_PushC:
                vectorPushValue(stack, vectorAtConst(constTable, instructionPtr->a));
                break;

            case IST_Reserve:
                // TODO
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
                break;

            case IST_DoubleVal:
                aVal = vectorAt(stack, stackPtr - 1);
                resVal = vectorAt(stack, stackPtr - 2);
                doubleval(aVal, resVal);
                break;

            case IST_FindString:
                bVal = vectorAt(stack, stackPtr - 1);
                aVal = vectorAt(stack, stackPtr - 2);
                resVal = vectorAt(stack, stackPtr - 3);
                findString(resVal, aVal, bVal);
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
                break;
            }

            case IST_IntVal:
                aVal = vectorAt(stack, stackPtr - 1);
                resVal = vectorAt(stack, stackPtr - 2);
                intval(aVal, resVal);
                break;

            case IST_PutString:
                aVal = vectorAt(stack, stackPtr - instructionPtr->a);
                resVal = vectorAt(stack, stackPtr - instructionPtr->a - 1);
                putString(resVal, aVal, instructionPtr->a);
                break;

            case IST_SortString:
                aVal = vectorAt(stack, stackPtr - 1);
                resVal = vectorAt(stack, stackPtr - 2);
                sortString(aVal, resVal);
                break;

            case IST_StrLen:
                aVal = vectorAt(stack, stackPtr - 1);
                resVal = vectorAt(stack, stackPtr - 2);
                strLen(aVal, resVal);
                break;

            case IST_StrVal:
                aVal = vectorAt(stack, stackPtr - 1);
                resVal = vectorAt(stack, stackPtr - 2);
                strval(aVal, resVal);
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
