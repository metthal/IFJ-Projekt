#include "interpreter.h"
#include "value_vector.h"
#include "builtin.h"
#include "nierr.h"

#include <stdint.h>

void interpret(const Instruction *firstInstruction, const Vector *constTable, const Vector *addressTable)
{
    // 3 value pointers used during interpretation
    Value *resVal, *aVal, *bVal;
    Vector *stack = newValueVector();

    // Create global stack frame
    // Reserve space for return value
    vectorPushDefaultValue(stack);
    // Null old stack pointer
    vectorPushDefaultValue(stack);
    resVal = vectorBack(stack);
    resVal->type = VT_InstructionPtr;
    resVal->data.ip = NULL;
    // Null old instruction pointer
    vectorPushDefaultValue(stack);
    resVal = vectorBack(stack);
    resVal->type = VT_StackPtr;
    resVal->data.sp = 0;


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

            case IST_Jmp:
                instructionPtr += instructionPtr->a;
                continue;

            case IST_Jmpz:
                bVal = vectorAt(stack, stackPtr + instructionPtr->b);

                // TODO convert bVal to boolean
                if (bVal)
                    instructionPtr += instructionPtr->a;
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

            case IST_Call:
                // TODO create stack frame
                instructionPtr = vectorAtConst(addressTable, instructionPtr->a);
                continue;

            case IST_Return:
                // Load old instruction pointer
                aVal = vectorAt(stack, stackPtr + 1);
                if (aVal->data.ip != NULL) {
                    instructionPtr = aVal->data.ip;

                }
                else {
                    // TODO end interpretation
                    return;
                }

                // Load old stack pointer
                aVal = vectorAt(stack, stackPtr);
                stackPtr = aVal->data.sp;

                // TODO resize to clear stack
                continue;

            case IST_Nullify:
                aVal = vectorAt(stack, stackPtr + instructionPtr->a);
                deleteValue(aVal);
                initValue(aVal);
                aVal->type = VT_Null;
                break;

                // TODO built-in functions

            default:
                break;
        }

        // Move to next instruction
        instructionPtr++;
    }
}
