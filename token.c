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

#include "token.h"
#include <stdlib.h>
#include <string.h>

Token* newToken()
{
    Token* tmp = malloc(sizeof(Token));
    memset(tmp, 0, sizeof(Token));
    tmp->type = STT_Empty;
    return tmp;
}

void initToken(Token *pt)
{
    memset(pt, 0, sizeof(Token));
    pt->type = STT_Empty;
}

void deleteToken(Token *pt)
{
    if (pt != NULL) {
        if ((pt->type == STT_Variable)
            || (pt->type == STT_Identifier)
            || (pt->type == STT_String)) {
            deleteString(&pt->str);
        }
    }
}

void freeToken(Token **ppt)
{
    if (ppt != NULL) {
        if (*ppt != NULL) {
            if (((*ppt)->type == STT_Variable)
                || ((*ppt)->type == STT_Identifier)
                || ((*ppt)->type == STT_String)) {
                deleteString(&(*ppt)->str);
            }
        }
        free(*ppt);
        *ppt = NULL;
    }
}

void copyToken(const Token *src, Token *dest)
{
    memset(dest, 0, sizeof(Token));
    dest->type = src->type;
    if(src->type == STT_Variable   ||
       src->type == STT_Identifier ||
       src->type == STT_String){
        copyString(&src->str, &dest->str);
    }
    else if(src->type == STT_Double)
        dest->d = src->d;
    else if(src->type == STT_Number)
        dest->n = src->n;
    else if(src->type == STT_Keyword)
        dest->keywordType = src->keywordType;
}
