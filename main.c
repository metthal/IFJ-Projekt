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

#include "nierr.h"
#include "rc.h"
#include "scanner.h"
#include "parser.h"

uint8_t processParams(int32_t argc, char **argv, Vector** tokenVector)
{
    if (argc != 2) {
        *tokenVector = NULL;
        return 1;
    }

    *tokenVector = scannerScanFile(argv[1]);
    return 0;
}

int main(int argc, char **argv)
{
    setError(ERR_None);

    Vector* tokenVector = NULL;
    if (processParams(argc, argv, &tokenVector))
        return RC_FatalError;

    if(getError())
        return getRcFromError();

    parse(tokenVector, 0);

    return getRcFromError();
}
