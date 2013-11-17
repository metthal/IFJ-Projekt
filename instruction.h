#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "token.h"

/** Instruction operand conventions:
 * Noop:
 * nothing
 *
 * Mov:
 * res - destination
 * a   - source
 *
 * Jmp:
 * a   - distance
 *
 * Jmpz:
 * a   - distance
 * b   - condition
 *
 * Push:
 * a   - item
 *
 * Reserve:
 * a   - count
 *
 * Pop:
 * a   - count
 *
 * Call:
 * a   - address index
 *
 * Return:
 * a   - parameter count
 *
 * Nullify:
 * a   - item
 *
 * Halt:
 * nothing
 */

typedef enum
{
    IST_Noop = 0,    //!< IST_Noop
    IST_Mov,         //!< IST_Mov
    IST_Jmp,         //!< IST_Jmp
    IST_Jmpz,        //!< IST_Jmpz
    IST_Push,        //!< IST_Push
    IST_Reserve,     //!< IST_Reserve
    IST_Pop,         //!< IST_Pop
    IST_Call,        //!< IST_Call
    IST_Return,      //!< IST_Return
    IST_Nullify,     //!< IST_Nullify
    IST_Halt,        //!< IST_Halt
    // Built-in functions instructions
    IST_BoolVal,     //!< IST_BoolVal
    IST_DoubleVal,   //!< IST_DoubleVal
    IST_FindString,  //!< IST_FindString
    IST_GetString,   //!< IST_GetString
    IST_GetSubstring,//!< IST_GetSubstring
    IST_IntVal,      //!< IST_IntVal
    IST_PutString,   //!< IST_PutString
    IST_SortString,  //!< IST_SortString
    IST_StrLen,      //!< IST_StrLen
    IST_StrVal,      //!< IST_StrVal
    // Helper instructions that aren't interpreted
    IST_Break,       //!< IST_Break
    IST_Continue     //!< IST_Continue
} InstructionCode;

typedef struct
{
    InstructionCode code;
    int32_t a;
    int32_t b;
    int32_t res;
} Instruction;

Instruction* newInstruction();

void initInstruction(Instruction *pt);

void deleteInstruction(Instruction *pt);

void freeInstruction(Instruction **ppt);

void copyInstruction(const Instruction *src, Instruction *dest);

void generateCall(const Token *id, uint32_t paramCount);

void generateInstruction(InstructionCode code, int32_t res, int32_t a, int32_t b);

uint32_t generateEmptyInstruction();

void fillInstruction(uint32_t index, InstructionCode code, int32_t res, int32_t a, int32_t b);

#endif
