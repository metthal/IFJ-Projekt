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
 * @file interpreter.h
 * @brief Declares interpreter.
 */

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "vector.h"
#include "instruction.h"

/**
 * Runs interpretation of generated instructions.
 * @param firstInstruction Pointer to first instruction of main (body) instruction vector.
 * @param constTable Table holding constants - their types and values.
 * @param addressTable Table holding addresses for calling functions.
 */
void interpret(const Instruction *firstInstruction, const Vector *constTable, const Vector *addressTable);

#endif
