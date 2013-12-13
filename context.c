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

#include "context.h"
#include "ial.h"
#include <stdlib.h>
#include <string.h>

void initContext(Context *pt)
{
    memset(pt, 0, sizeof(Context));
    pt->localTable = newSymbolTable();
}

void deleteContext(Context *pt)
{
    if (pt != NULL) {
        freeSymbolTable(&pt->localTable);
    }
}
