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

struct mcctype * typeClone( struct mcctype * t )
{
    struct mcctype * out = sbrk(sizeof(struct mcctype));
    if ( out == SBRKFAIL ) mccfail("unable to allocate new type");

    out->ptype = t->ptype;
    out->stype = t->stype;

    struct mccsubtype * s, ** os = &out->sub;
    for ( s = t->sub; s; s = s->sub )
    {
        *os = sbrk(sizeof(struct mccsubtype));
        if ( *os == SBRKFAIL ) mccfail("unable to allocate new subtype");

        (*os)->inder = s->inder;
        (*os)->arrays = s->arrays;

        (*os)->sizes = sbrk(s->arrays);
        if ( (*os)->sizes == SBRKFAIL ) mccfail("unable to allocate new size list");

        int16_t i;
        for ( i = 0; i < s->arrays; i++ ) (*os)->sizes[i] = s->sizes[i];

        (*os)->ftype = s->ftype; // This should be safe since function namespaces are also immutable

        os = &(*os)->sub;
    }

    return out;
}

// Inderect to type
struct mcctype * typeInder( struct mcctype * t )
{
    struct mcctype * out = typeClone(t);

    // Get active subtype
    struct mccsubtype * s;
    for ( s = out->sub; s->sub; s = s->sub );

    if ( s->arrays || s->ftype )
    {
        s = s->sub = sbrk(sizeof(struct mccsubtype));
        if ( s == SBRKFAIL ) mccfail("unable to allocate new subtype");

        s->inder = 1;
        s->arrays = 0;
        s->sizes = NULL;
        s->ftype = NULL;
        s->sub = NULL;
    }
    else s->inder++;

    return out;
}

// Dereference type
struct mcctype * typeDeref( struct mcctype * t )
{
    struct mcctype * out = typeClone(t);

    struct mccsubtype * s, * ls = NULL;
    for ( s = out->sub; s->sub; s = s->sub ) ls = s;

    if ( s->ftype ) mccfail("tried to dereference a function");
    else if ( s->arrays )
    {
        s->arrays--;

        int16_t i;
        for ( i = 0; i < s->arrays; i++ ) s->sizes[i] = s->sizes[i+1];
    }
    else if ( s->inder ) s->inder--;
    else mccfail("tried to dereference non-pointer");

    // Reduce type if needed
    if ( ls && !s->inder && !s->arrays && !s->ftype ) ls->sub = NULL;

    return out;
}

// Size required to store type in bytes
int16_t typeSize( struct mcctype * t )
{
    // Get active subtype
    struct mccsubtype * s;
    for ( s = t->sub; s->sub; s = s->sub );

    if ( s->ftype ) mccfail("cannot get sizeof function");

    // Check if this is an array
    int16_t i, j;
    for ( i = 0, j = 1; i < s->arrays; i++ )
    {
        if ( !s->sizes[i] ) mccfail("cannot get sizeof unknown dimension in array");
        j *= s->sizes[i];
    }
    if ( i )
    {
        void * drbp = sbrk(0);

        s->arrays = 0; // Temporarily drop all arrays

        j *= typeSize(typeClone(t));

        s->arrays = i;

        brk(drbp);

        return j;
    }

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


// Prote arithmetic types
struct mcctype * typePromote( struct mcctype * ta, struct mcctype * tb )
{
    unsigned int8_t pa = ta->ptype & CPL_DTYPE_MASK;
    unsigned int8_t pb = tb->ptype & CPL_DTYPE_MASK;

    return pa >= pb ? ta : tb;
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

int16_t isCompatible( struct mcctype * ta, struct mcctype * tb )
{
    // These values may be invalid, confirm first
    unsigned int8_t pa = ta->ptype & CPL_DTYPE_MASK;
    unsigned int8_t pb = tb->ptype & CPL_DTYPE_MASK;

    // Pointer, must be exact match (or void pointer)
    if ( isPointer(ta) )
    {
        if ( !isPointer(tb) ) return 0; // Pointer missmatch

        // Void pointers are compatible with everything
        if ( !ta->stype && pa == CPL_VOID || !tb->stype && pb == CPL_VOID ) return 1;

        // Struct types don't match or primative types don't match
        if ( ta->stype != tb->stype || (!ta->stype && pa != pb) ) return 0;

        // Struct must be complete
        if ( ta->stype && (~ta->stype->type & CPL_DEFN) ) mccfail("incomplete type is incompatible");

        struct mccsubtype * sa, * sb;
        for ( sa = ta->sub, sb = tb->sub; sa && sb; sa = sa->sub, sb = sb->sub )
        {
            if ( sa->inder + sa->arrays != sb->inder + sb->arrays ) return 0; // Inderection missmatch

            if ( sa->ftype )
            {
                if ( !sb->ftype ) return 0;

                // TODO ensure function definitions match and that functions are complete
            }
        }

        // Subtype count missmatch
        if ( sa || sb ) return 0;

        return 1;
    }
    else if ( isPointer(tb) ) return 0; // Pointer missmatch

    // If either is a structure, make sure they're compatible
    if ( ta->stype != tb->stype ) return 0;

    // Both types are structs and match
    if ( ta->stype )
    {
        if ( ~ta->stype->type & CPL_DEFN ) mccfail("incomplete type is incompatible");
        return 1;
    }

    // Ensure primative types are compatible (void is only compatible with self)
    if ( (pa == CPL_VOID) != (pb == CPL_VOID) ) return 0;

    if ( pa < pb ) return -1; // PB won't fit into PA

    return 1;
}