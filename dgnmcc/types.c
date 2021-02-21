struct mccsubtype subtype_val = {
    0, 0, NULL, NULL, NULL
};

struct mccsubtype subtype_ptr = {
    1, 0, NULL, NULL, NULL
};

struct mcctype type_char = {
    CPL_CHR,
    NULL,
    &subtype_val
};

struct mcctype type_int = {
    CPL_INT,
    NULL,
    &subtype_val
};

struct mcctype type_long = {
    CPL_LNG,
    NULL,
    &subtype_val
};

struct mcctype type_string = {
    CPL_CHR,
    NULL,
    &subtype_ptr
};

// Size required to store type in bytes
int16_t typeSize( struct mcctype * t )
{
    // Get active subtype
    struct mccsubtype * s;
    for ( s = t->sub; s->sub; s = s->sub );

    if ( s->ftype ) mccfail("cannot get sizeof function");

    // Check if this is an array
    int16_t i, j;
    for ( i = 0, j = 0; i < s->arrays; i++ )
    {
        if ( !s->sizes[i] ) mccfail("cannot get sizeof unknown dimension in array");
        j *= s->sizes[i];
    }
    if ( i ) return j;

    // Check if this is a pointer
    if ( s->inder ) return 2;

    // Check if this is a struct
    if ( t->stype )
    {
        if ( ~t->stype->type & CPL_DEFN ) mccfail("tried to get sizeof incomplete type");
        return t->stype->size;
    }

    // Must be a primative type
    int8_t ptype = t->ptype & CPL_DTYPE_MASK;

    // Can't get size of void
    if ( ptype == CPL_VOID ) mccfail("tried to get sizeof void");

    if ( ptype <= CPL_UCHR ) return 1;
    if ( ptype <= CPL_UINT ) return 2;
    if ( ptype <= CPL_FPV  ) return 4;

    return 8; // Double
}

/*
    CPL Type hierarchy:

    scalar > arith(metic) > integer

    integer = char, int, long
    arith(metic) = integer, float
    scalar = arith(metic), pointer
*/

int16_t isInteger( struct mcctype * t )
{
    struct mccsubtype * s;
    for ( s = t->sub; s->sub; s = s->sub );

    unsigned int8_t p = t->ptype & CPL_DTYPE_MASK;

    return !(s->ftype || s->inder || s->arrays || t->stype ) && p && p <= CPL_ULNG;
}

int16_t isArith( struct mcctype * t )
{
    struct mccsubtype * s;
    for ( s = t->sub; s->sub; s = s->sub );

    return !(s->ftype || s->inder || s->arrays || t->stype ) && t->ptype & CPL_DTYPE_MASK;
}

int16_t isPointer( struct mcctype * t )
{
    struct mccsubtype * s;
    for ( s = t->sub; s->sub; s = s->sub );

    return !s->ftype && (s->inder || s->arrays);
}

int16_t isScalar( struct mcctype * t )
{
    struct mccsubtype * s;
    for ( s = t->sub; s->sub; s = s->sub );

    return !s->ftype && ( s->inder || s->arrays || !t->stype ) && t->ptype & CPL_DTYPE_MASK;
}

int16_t isFunction( struct mcctype * t )
{
    // Get active subtype
    struct mccsubtype * s;
    for ( s = t->sub; s->sub; s = s->sub );

    return s->ftype == NULL;
}

int16_t isStruct( struct mcctype * t )
{
    return t->stype && !isFunction(t);
}

int16_t isArray( struct mcctype * t )
{
    struct mccsubtype * s;
    for ( s = t->sub; s->sub; s = s->sub );

    return s->arrays;
}

int16_t isCompatible( struct mcctype * a, struct mcctype * b )
{
    // TODO check type compatibility
}
