// Search for symbol in immediate namespace
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

    return NULL;
}

// Search for symbol in all scoped namespaces
struct mccsym * findSymbol( struct mccnsp * sernsp, int8_t * name, int16_t len )
{
    struct mccsym * cursym;

    while ( sernsp )
    {
        // Found symbol
        if ( cursym = getSymbol( sernsp, name, len ) ) return cursym;

        // TODO search for symbol in anonymous structs, unions, etc...

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

int16_t compareSymbol( struct mccsym * sa, struct mccsym * sb )
{
    return 1;
}
