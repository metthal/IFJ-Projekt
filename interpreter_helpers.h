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
 * @file interpreter_helpers.h
 * @brief Declares helper functions used in interpret.
 */

#ifndef INTERPRETER_HELPERS_H
#define INTERPRETER_HELPERS_H

// Just performance optimizations, improves the speed by ~4% by inlining

/**
 * @brief Loads res with correct address.
 * @param base Stack pointer from which to offset.
 * @param res A place to hold address.
 * @param ires Index of address.
 */
static inline void fillResValuePtr(Value *base, Value **res, int32_t ires)
{
    *res = base + ires;
    if ((*res)->type == VT_StrongReference)
        *res += (*res)->data.ref;
}

/**
 * @brief Loads x with correct address.
 * @param base Stack pointer from which to offset.
 * @param constBase Constants pointer from which to offset.
 * @param x A place to hold address.
 * @param ix Index of address.
 * @return 1 if successful.
 */
static inline uint8_t fillConstValuePtr(Value *base, const Value *constBase, const Value **x,
        int32_t ix)
{
    *x = base + ix;
    switch ((*x)->type){
        case VT_StrongReference:
        case VT_WeakReference:
            *x += (*x)->data.ref;
            break;
        case VT_ConstReference:
            *x = constBase + (*x)->data.ref;
            break;
        case VT_Undefined:
            setError(ERR_UndefVariable);
            return 0;
        default:
            break;
    }

    return 1;
}

/**
 * @brief Loads res and x with correct address.
 * @param base Stack pointer from which to offset.
 * @param constBase Constants pointer from which to offset.
 * @param res A place to hold address for res.
 * @param x A place to hold address for x.
 * @param ires Index of res address.
 * @param ix Index of x address.
 * @return 1 if successful.
 */
static inline uint8_t fillResConstValuePtr(Value *base, const Value *constBase, Value **res, const Value **x,
        int32_t ires, int32_t ix)
{
    *res = base + ires;
    if ((*res)->type == VT_StrongReference)
        *res += (*res)->data.ref;

    *x = base + ix;
    switch ((*x)->type){
        case VT_StrongReference:
        case VT_WeakReference:
            *x += (*x)->data.ref;
            break;
        case VT_ConstReference:
            *x = constBase + (*x)->data.ref;
            break;
        case VT_Undefined:
            setError(ERR_UndefVariable);
            return 0;
        default:
            break;
    }

    return 1;
}

/**
 * @brief Loads A and B with correct address.
 * @param base Stack pointer from which to offset.
 * @param constBase Constants pointer from which to offset.
 * @param a A place to hold address for A.
 * @param b A place to hold address for B.
 * @param ia Index of A address.
 * @param ib Index of B address.
 * @return 1 if successful.
 */
static inline uint8_t fillConstValuePtrs(Value *base, const Value *constBase, const Value **a, const Value **b,
        int32_t ia, int32_t ib)
{
    *a = base + ia;
    switch ((*a)->type){
        case VT_StrongReference:
        case VT_WeakReference:
            *a += (*a)->data.ref;
            break;
        case VT_ConstReference:
            *a = constBase + (*a)->data.ref;
            break;
        case VT_Undefined:
            setError(ERR_UndefVariable);
            return 0;
        default:
            break;
    }

    *b = base + ib;
    switch ((*b)->type){
        case VT_StrongReference:
        case VT_WeakReference:
            *b += (*b)->data.ref;
            break;
        case VT_ConstReference:
            *b = constBase + (*b)->data.ref;
            break;
        case VT_Undefined:
            setError(ERR_UndefVariable);
            return 0;
        default:
            break;
    }

    return 1;
}

/**
 * @brief Loads A, B and res with correct address.
 * @param base Stack pointer from which to offset.
 * @param constBase Constants pointer from which to offset.
 * @param res A place to hold address for res.
 * @param a A place to hold address for A.
 * @param b A place to hold address for B.
 * @param ires Index of res address.
 * @param ia Index of A address.
 * @param ib Index of B address.
 * @return 1 if successful.
 */
static inline uint8_t fillValuePtrs(Value *base, const Value *constBase, Value **res, const Value **a, const Value **b,
        const Instruction *iptr)
{
    *res = base + iptr->res;
    if ((*res)->type == VT_StrongReference)
        *res += (*res)->data.ref;

    if (iptr->mode & ISM_FirstConst)
        *a = constBase + iptr->a;
    else {
        *a = base + iptr->a;
        switch ((*a)->type){
            case VT_StrongReference:
            case VT_WeakReference:
                *a += (*a)->data.ref;
                break;
            case VT_ConstReference:
                *a = constBase + (*a)->data.ref;
                break;
            case VT_Undefined:
                setError(ERR_UndefVariable);
                return 0;
            default:
                break;
        }
    }

    if (iptr->mode & ISM_SecondConst)
        *b = constBase + iptr->b;
    else {
        *b = base + iptr->b;
        switch ((*b)->type){
            case VT_StrongReference:
            case VT_WeakReference:
                *b += (*b)->data.ref;
                break;
            case VT_ConstReference:
                *b = constBase + (*b)->data.ref;
                break;
            case VT_Undefined:
                setError(ERR_UndefVariable);
                return 0;
            default:
                break;
        }
    }

    return 1;
}

#endif
