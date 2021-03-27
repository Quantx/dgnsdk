// Search for symbol in immediate and child anonymous namespaces
struct mccsym * getSymbol( struct mccnsp * sernsp, int8_t * name, int16_t len )
{
    struct mccsym * cursym;

    for ( cursym = sernsp->symtbl; cursym; cursym = cursym->next )
    {
        if ( len != cursym->len ) continue;

        int16_t i;
        for ( i = 0; i < len && cursym->name[i] == name[i]; i++ );
        if ( i == len ) return cursym;
    }

    struct mccnsp * curnsp;

    for ( curnsp = sernsp->nsptbl; curnsp; curnsp = curnsp->next )
    {
        unsigned int8_t nsptype = curnsp->type & CPL_NSPACE_MASK;

        // Search child anonymous namespaces and enums
        if ( nsptype == CPL_ENUM || ((nsptype == CPL_STRC || nsptype == CPL_UNION) && !curnsp->name && curnsp->type & CPL_DEFN && curnsp->type & CPL_INST ) )
        {
            cursym = getSymbol( curnsp, name, len );
            if ( cursym ) return cursym;
        }
    }

    return NULL;
}

// Search for symbol in each parent namespace
struct mccsym * findSymbol( struct mccnsp * sernsp, int8_t * name, int16_t len )
{
    struct mccsym * cursym;

    while ( sernsp )
    {
        // Found symbol
        if ( cursym = getSymbol( sernsp, name, len ) ) return cursym;

        // Check parent namespace
        sernsp = sernsp->parent;
    }

    return NULL;
}

struct mccnsp * getNamespace( struct mccnsp * sernsp, int8_t * name, int16_t len )
{
    struct mccnsp * curnsp;

    for ( curnsp = sernsp->nsptbl; curnsp; curnsp = curnsp->next )
    {
        if ( len != curnsp->len ) continue;

        int16_t i;
        for ( i = 0; i < len && curnsp->name[i] == name[i]; i++ );
        if ( i == len ) return curnsp;
    }

    return NULL;
}

// Search for symbol in each parent namespace
struct mccnsp * findNamespace( struct mccnsp * sernsp, int8_t * name, int16_t len )
{
    while ( sernsp )
    {
        struct mccnsp * curnsp;
        // Found symbol
        if ( curnsp = getNamespace( sernsp, name, len ) ) return curnsp;

        // Check parent namespace
        sernsp = sernsp->parent;
    }

    return NULL;
}
