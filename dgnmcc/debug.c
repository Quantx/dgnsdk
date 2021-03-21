void writeType( int16_t fd, struct mcctype * t, int8_t * name, int16_t len )
{
    if ( !t ) return;

    if ( t->stype ) // Struct, union, enum
    {
        int8_t st = t->stype->type & CPL_NSPACE_MASK;

        int8_t * stn = namespaceNames[st];
        int16_t stl = 0;
        while ( stn[stl] ) stl++;

        write( fd, stn, stl );

        write( fd, " ", 1 );

        write( fd, t->stype->name, t->stype->len );
    }
    else // Primative type
    {
        int8_t * ptn = typeNames[t->ptype & CPL_NSPACE_MASK];
        int16_t ptl = 0;
        while ( ptn[ptl] ) ptl++;

        write( fd, ptn, ptl );
    }

    write( fd, " ", 1 );

    struct mccsubtype * sstk[32];
    unsigned int16_t stop = 0;

    int16_t i;
    struct mccsubtype * s;
    for ( s = t->sub; s; s = s->sub )
    {
        sstk[stop++] = s; // Push to stack

        // Print inderects
        for ( i = 0; i < s->inder; i++ ) write( fd, "*", 1 );

        if ( s->sub ) write( fd, "(", 1 );
    }

    if ( len ) write( fd, name, len );
    else write( fd, "?", 1 );

    while ( stop )
    {
        s = sstk[stop - 1];

        if ( s->ftype )
        {
            write( fd, "( ", 2);

            struct mccsym * cursym;
            for ( cursym = s->ftype->symtbl; cursym; cursym = cursym->next )
            {
                writeType( fd, &cursym->type, cursym->name, cursym->len );

                write( fd, " @ ", 3 );
                decwrite( fd, cursym->addr );
                if ( cursym->next ) write( fd, ", ", 2 );
            }

            write( fd, ")\n", 2 );
        }

        for ( i = 0; i < s->arrays; i++ )
        {
            write( fd, "[", 1 );
            decwrite( fd, s->sizes[i] );
            write( fd, "]", 1 );
        }

        if ( --stop ) write( fd, ")", 1 );
    }
}

void writeToken( int16_t fd, unsigned int8_t tokn )
{
    if ( tokn >= 128 )
    {
        int8_t * opn = tokenNames[tokn - 128];
        int16_t opl = 0;
        while ( opn[opl] ) opl++;

        write( fd, opn, opl );
    }
    else if ( tokn >= 32 ) write( fd, &tokn, 1 );
    else decwrite( fd, tokn );
}

void dumpTree( struct mccnode * n, int8_t * fname )
{
    struct mccnode * ns[MAX_EXPR_NODE];
    int16_t nspos = 0;

    int16_t fexp;
    if ( (fexp = creat(fname, 0666)) < 0 ) mccfail( "unable to create tree dump file" );

    write( fexp, "graph expression_tree {\n", 24 );

    while ( n || nspos )
    {
        if ( !n )
        {
            n = ns[--nspos]->right;
            continue;
        }

        write( fexp, "\"", 1 );
        decwrite( fexp, (int16_t)(long)n );
        write( fexp, "\" [label=\"", 10 );

        if ( n->oper == Variable ) writeType( fexp, n->type, n->sym->name, n->sym->len );
        else writeType( fexp, n->type, NULL, 0 );
        write( fexp, "\\n", 2 );

        if ( n->oper == Named )
        {
            write( fexp, "\\\"", 2 );
            write( fexp, n->name, n->val );
            write( fexp, "\\\"", 2 );
        }
        else if ( n->oper == Number || n->oper == SmolNumber ) decwrite( fexp, n->val );
        else if ( n->oper == LongNumber ) octwrite( fexp, n->valLong );
        else writeToken( fexp, n->oper );

        write( fexp, "\"];\n", 4 );


        if ( n->left )
        {
            write( fexp, "\"", 1 );
            decwrite( fexp, (int16_t)(long)n );
            write( fexp, "\" -- \"", 6 );
            decwrite( fexp, (int16_t)(long)n->left );
            write( fexp, "\";\n", 3 );
        }

        if ( n->right )
        {
            write( fexp, "\"", 1 );
            decwrite( fexp, (int16_t)(long)n );
            write( fexp, "\" -- \"", 6 );
            decwrite( fexp, (int16_t)(long)n->right );
            write( fexp, "\";\n", 3 );
        }

        ns[nspos++] = n;
        n = n->left;
    }

    write( fexp, "}", 1 );
    close( fexp );
}

void dumpNamespace( int16_t fd, struct mccnsp * dmpnsp )
{
    int8_t st = dmpnsp->type & CPL_NSPACE_MASK;

    int8_t * stn = namespaceNames[st];
    int16_t stl = 0;
    while ( stn[stl] ) stl++;

    write( fd, stn, stl );

    if ( dmpnsp->len )
    {
        write( fd, " ", 1 );
        write( fd, dmpnsp->name, dmpnsp->len );
    }

    if ( dmpnsp->type & CPL_DEFN )
    {
        if ( st == CPL_FUNC || st == CPL_VFUNC ) write( fd, "( ", 2 );
        else write( fd, " {\n", 3 );

        struct mccsym * cursym;
        for ( cursym = dmpnsp->symtbl; cursym; cursym = cursym->next )
        {
            writeType( fd, &cursym->type, cursym->name, cursym->len );

            write( fd, " @ ", 3 );
            decwrite( fd, cursym->addr );

            if ( (st == CPL_FUNC || st == CPL_VFUNC) && cursym->next ) write( fd, ", ", 2 );
            else write( fd, ";\n", 2 );
        }

        if ( st == CPL_FUNC || st == CPL_VFUNC ) write( fd, ");", 2 );
        else write( fd, "};\n", 3 );
    }
    else write( fd, ";\n", 2 );

    struct mccnsp * curnsp;
    for ( curnsp = dmpnsp->nsptbl; curnsp; curnsp = curnsp->next ) dumpNamespace( fd, curnsp );
}

// Dump the global namespace to a file
void dumpGlbnsp( int8_t * fname )
{
    int16_t fnsp;
    if ( (fnsp = creat(fname, 0666)) < 0 ) mccfail( "unable to create namespace dump file" );

    dumpNamespace( fnsp, &glbnsp );

    close(fnsp);
}
