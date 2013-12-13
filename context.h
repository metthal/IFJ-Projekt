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
 * @file context.h
 * @brief Declares context that represents most of parser's state.
 */

#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdint.h>

struct SymbolTable;

/** Structure that holds data of Context */
typedef struct {
    struct SymbolTable *localTable;
    uint32_t defaultStart; //!< Starting index in Constants Table
    uint32_t exprStart; //!< First expression index in stack
    uint16_t argumentCount;
    uint16_t localVariableCount;
    uint16_t defaultCount;
} Context;

/**
 * @brief Initializes context.
 * @param pt Context to initialize.
 */
void initContext(Context *pt);

/**
 * @brief Deletes context.
 * @param pt Context to delete.
 */
void deleteContext(Context *pt);

#endif
