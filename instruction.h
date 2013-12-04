#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "builtin.h"
#include "token.h"
#include "symbol.h"

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
 * res - exprStart
 * a   - distance
 * b   - condition
 *
 * Push:
 * a   - item
 *
 * PushC:
 * a   - constants table index
 *
 * Reserve:
 * a   - count
 *
 * Pop:
 * a   - count
 *
 * ClearExpr:
 * a   - exprStart
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
 */

typedef enum
{
    IST_Noop = 0,    //!< IST_Noop
    IST_Mov,         //!< IST_Mov
    IST_MovC,        //!< IST_MovC
    IST_Jmp,         //!< IST_Jmp
    IST_Jmpz,        //!< IST_Jmpz
    IST_Jmpnz,       //!< IST_Jmpnz
    IST_Push,        //!< IST_Push
    IST_PushC,       //!< IST_PushC
    IST_PushRef,     //!< IST_PushRef
    IST_Reserve,     //!< IST_Reserve
    IST_Pop,         //!< IST_Pop
    IST_ClearExpr,   //!< IST_ClearExpr
    IST_Call,        //!< IST_Call
    IST_Return,      //!< IST_Return
    IST_Nullify,     //!< IST_Nullify
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
    IST_Continue,    //!< IST_Continue
    // Expression related instructions
    IST_Add,         //!< IST_Add
    IST_Subtract,    //!< IST_Subtract
    IST_Multiply,    //!< IST_Multiply
    IST_Divide,      //!< IST_Divide
    IST_Concat,      //!< IST_Concat
    IST_Equal,       //!< IST_Equal
    IST_NotEqual,    //!< IST_NotEqual
    IST_Less,        //!< IST_Less
    IST_LessEq,      //!< IST_LessEq
    IST_Greater,     //!< IST_Greater
    IST_GreaterEq,   //!< IST_GreaterEq
    IST_And,         //!< IST_And
    IST_Or,          //!< IST_Or
    IST_Not          //!< IST_Not
} InstructionCode;

typedef enum
{
    ISM_NoConst     = 0,
    ISM_FirstConst  = 1,
    ISM_SecondConst = 2,
    ISM_AllConst    = 3
} InstructionMode;

typedef struct
{
    InstructionCode code;
    InstructionMode mode;
    int32_t a;
    int32_t b;
    int32_t res;
} Instruction;

Instruction* newInstruction();

void initInstruction(Instruction *pt);

void deleteInstruction(Instruction *pt);

void freeInstruction(Instruction **ppt);

void copyInstruction(const Instruction *src, Instruction *dest);

uint32_t generateCall(const Symbol *symbol, BuiltinCode builtinCode, uint32_t paramCount);

void generateInstruction(InstructionCode code, InstructionMode mode, int32_t res, int32_t a, int32_t b);

uint32_t generateEmptyInstruction();

void fillInstruction(uint32_t index, InstructionCode code, int32_t res, int32_t a, int32_t b);

Symbol* fillInstFuncInfo(const Token *funcToken, BuiltinCode *builtinCode, int64_t *paramCount);

#endif
