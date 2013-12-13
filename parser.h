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
 * @file parser.h
 * @brief Declares top-down parser.
 */

#ifndef PARSER_H
#define PARSER_H

#include "vector.h"

/**
 * @brief Parses vector of tokens to create interpretable program.
 * @param tokenVector Vector of tokens representing file to parse.
 * @param testRun Internal parameter for unit testing.
 */
void parse(Vector* tokenVector, uint8_t testRun);

#endif
