#ifndef INTERPRETER_HELPERS_H
#define INTERPRETER_HELPERS_H

// Just performance optimizations, improves the speed by ~4% by inlining

static inline void fillResValuePtr(Value *base, Value **res, int32_t ires)
{
    *res = base + ires;
    if ((*res)->type == VT_StrongReference)
        *res += (*res)->data.ref;
}

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
