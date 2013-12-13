/*
 * Project name:
 * Implementace interpretu imperativního jazyka IFJ13.
 *
 * Codename:
 * INI: Ni Interpreter
 *
 * Description:
 * https://wis.fit.vutbr.cz/FIT/st/course-files-st.php/course/IFJ-IT/projects/ifj2013.pdf
 *
 * Project's GitHub repository:
 * https://github.com/metthal/IFJ-Projekt
 *
 * Team:
 * Marek Milkovič   (xmilko01)
 * Lukáš Vrabec     (xvrabe07)
 * Ján Spišiak      (xspisi03)
 * Ivan Ševčík      (xsevci50)
 * Marek Bertovič   (xberto00)
 */

/**
 * @file instruction.h
 * @brief Declares all instruction related stuff.
 */

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

/** Instruction modes. */
typedef enum
{
    ISM_NoConst     = 0,
    ISM_FirstConst  = 1,
    ISM_SecondConst = 2,
    ISM_AllConst    = 3
} InstructionMode;

/** Structure that holds data of Instruction. */
typedef struct
{
    InstructionCode code;
    InstructionMode mode;
    int32_t a;
    int32_t b;
    int32_t res;
} Instruction;

/**
 * @brief Creates new instruction on heap.
 * @return Pointer to created instruction.
 */
Instruction* newInstruction();

/**
 * @brief Initializes existing instruction.
 * @param pt Instruction to initialize.
 */
void initInstruction(Instruction *pt);

/**
 * @brief Deletes existing instruction.
 * @param pt Instruction to delete.
 */
void deleteInstruction(Instruction *pt);

/**
 * @brief Frees an instruction created on heap.
 * @param ppt Instruction to free.
 */
void freeInstruction(Instruction **ppt);

/**
 * @brief Makes a copy of an instruction.
 * @param src Source instruction used as image.
 * @param dest Destination instruction to be filled.
 */
void copyInstruction(const Instruction *src, Instruction *dest);

/**
 * @brief Makes checks and generates a function call instruction.
 * @param symbol Symbol that holds informations about function.
 * @param builtinCode Code of built-in function to be called.
 * @param paramCount Number of parameters with which to call.
 * @return
 */
uint32_t generateCall(const Symbol *symbol, BuiltinCode builtinCode, uint32_t paramCount);

/**
 * @brief Generates most of instructions.
 * @param code Code of instruction to generate. Can't be IST_Call.
 * @param mode Instruction mode.
 * @param res Result index for instruction.
 * @param a A operand index for instruction.
 * @param b B operand index for instruction.
 */
void generateInstruction(InstructionCode code, InstructionMode mode, int32_t res, int32_t a, int32_t b);

/**
 * @brief Generates an empty no-op instruction that can be filled later.
 * @return Index of generated instruction in instruction vector.
 */
uint32_t generateEmptyInstruction();

/**
 * @brief Fills pre-generated empty instruction.
 * @param index Index of instruction to fill.
 * @param code Code of instruction.
 * @param res Result index for instruction.
 * @param a A operand index for instruction.
 * @param b B operand index for instruction.
 */
void fillInstruction(uint32_t index, InstructionCode code, int32_t res, int32_t a, int32_t b);

/**
 * @brief Returns informations about function using token from scanner.
 * @param funcToken Token holding function's informations.
 * @param builtinCode If token represents built-in function, it's code is set here.
 * @param paramCount Number of function's parameters.
 * @return Symbol that golds informations about function.
 */
Symbol* fillInstFuncInfo(const Token *funcToken, BuiltinCode *builtinCode, int64_t *paramCount);

#endif
