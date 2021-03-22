void statement( struct mccnsp * curnsp, int sws )
{
    if ( tk == ';' ) ntok();
    else if ( tk == '{' )
    {
        struct mccnsp * cbnsp = sbrk(sizeof(struct mccnsp));
        if ( cbnsp == SBRKFAIL ) mccfail("unable to allocate room for child block");

        cbnsp->name = NULL;
        cbnsp->len = 0;

        cbnsp->type = CPL_BLOCK;
        cbnsp->size = curnsp->size; // Inherit offset

        cbnsp->symtbl = NULL; cbnsp->symtail = &cbnsp->symtbl;
        cbnsp->nsptbl = NULL; cbnsp->nsptail = &cbnsp->nsptbl;

        cbnsp->parent = curnsp;
        cbnsp->next = NULL;

        // Add to parent namespace
        *curnsp->nsptail = cbnsp;
        curnsp->nsptail = &cbnsp->next;

        ntok();
        while ( tk != '}' ) statement( cbnsp, sws );
        ntok();

        // Drop child from parent namespace table
        struct mccnsp ** cnsp;
        int16_t i = 0;
        for ( cnsp = &curnsp->nsptbl; *cnsp && *cnsp != cbnsp; cnsp = &(*cnsp)->next )i++;

#if DEBUG
        if ( !(*cnsp) ) mccfail( "cnsp is null" );
#endif

        // Make sure namespace tail doesn't become detached
        if ( curnsp->nsptail == &(*cnsp)->next ) curnsp->nsptail = cnsp;

        *cnsp = (*cnsp)->next; // Drop current namespace

#if DEBUG
        if ( curnsp->nsptail != &curnsp->nsptbl ) mccfail("child block tail dropped");
#endif

        // Unallocate child block
        brk(cbnsp);
    }
    else if ( tk == If )
    {
        write( segs[SEG_TEXT], "\tIF\n", 4 );

        ntok();
        if ( tk != '(' ) mccfail( "missing opening parenthasis in if statement" );
        ntok();

        void * erbp = sbrk(0);

        struct mccnode * root = expr( curnsp, ')' );

        if ( tk != ')' ) mccfail( "missing closing parenthasis in if statement" );
        ntok();

        emit(curnsp, root);
        brk(erbp);

        statement(curnsp, sws);

        if ( tk == Else )
        {
            write( segs[SEG_TEXT], "\tELSE\n", 6 );

            ntok();
            statement(curnsp, sws);
        }

        write( segs[SEG_TEXT], "\tIF_END\n", 8 );
    }
    else if ( tk == Switch )
    {
        write( segs[SEG_TEXT], "\tSWITCH\n", 8 );

        ntok();
        if ( tk != '(' ) mccfail( "missing opening parenthasis in switch statement" );
        ntok();

        void * erbp = sbrk(0);

        struct mccnode * root = expr( curnsp, ')' );

        if ( tk != ')' ) mccfail( "missing closing parenthasis in switch statement" );
        ntok();

        emit(curnsp, root);
        brk(erbp);

        statement(curnsp, 1);

        write( segs[SEG_TEXT], "\tEND_SWITCH\n", 12 );
    }
    else if ( tk == Case )
    {
        write( segs[SEG_TEXT], "\tCASE\n", 6 );

        if ( !sws ) mccfail("case statement outside of switch statement");

        void * erbp = sbrk(0);

        struct mccnode * root = expr( curnsp, ':' );

        if ( tk != ':' ) mccfail( "missing colon in case statement" );
        ntok();

        emit(curnsp, root);
        brk(erbp);

        // TODO case requires constant expression
    }
    else if ( tk == Default )
    {
        write( segs[SEG_TEXT], "\tDEFAULT\n", 9 );

        if ( !sws ) mccfail("default statement outside of switch statement");

        ntok();
        if ( tk != ':' ) mccfail( "missing colon after default statement" );
        ntok();
    }
    else if ( tk == Break )
    {
        write( segs[SEG_TEXT], "\tBREAK\n", 7 );

        ntok();
        if ( tk != ';' ) mccfail( "missing semicolon after break" );
        ntok();
    }
    else if ( tk == Continue )
    {
        write( segs[SEG_TEXT], "\tCONTINUE\n", 10 );

        ntok();
        if ( tk != ';' ) mccfail( "missing semicolon after continue" );
        ntok();
    }
    else if ( tk == Return )
    {
        write( segs[SEG_TEXT], "\tRETURN\n", 8 );

        // TODO return statement
    }
    else if ( tk == For )
    {
        write( segs[SEG_TEXT], "\tFOR\n", 5 );

        ntok();
        if ( tk != '(' ) mccfail( "missing opening parenthasis in for statement" );
        ntok();

        void * erbp;
        struct mccnode * root;

        // Init expression

        erbp = sbrk(0);

        root = expr( curnsp, ';' );

        if ( tk != ';' ) mccfail( "missing first semicolon in for statement" );
        ntok();

        emit(curnsp, root);
        brk(erbp);

        // Eval expression

        erbp = sbrk(0);

        root = expr( curnsp, ';' );

        if ( tk != ';' ) mccfail( "missing second semicolon in for statement" );
        ntok();

        emit(curnsp, root);
        brk(erbp);

        // Iterator expression

        erbp = sbrk(0);

        root = expr( curnsp, ';' );

        if ( tk != ')' ) mccfail( "missing closing parenthasis in for statement" );
        ntok();

        emit(curnsp, root);
        brk(erbp);

        // Statement

        statement(curnsp, sws);

        write( segs[SEG_TEXT], "\tEND_FOR\n", 9 );
    }
    else if ( tk == While )
    {
        write( segs[SEG_TEXT], "\tWHILE\n", 7 );

        ntok();
        if ( tk != '(' ) mccfail( "missing opening parenthasis in while statement" );
        ntok();

        void * erbp = sbrk(0);

        struct mccnode * root = expr( curnsp, ')' );

        if ( tk != ')' ) mccfail( "missing closing parenthasis in while statement" );
        ntok();

        emit(curnsp, root);
        brk(erbp);

        statement(curnsp, sws);

        write( segs[SEG_TEXT], "\tEND_WHILE\n", 11 );
    }
    else if ( tk == Do )
    {
        write( segs[SEG_TEXT], "\tDO\n", 4 );

        ntok();
        statement(curnsp, sws);

        write( segs[SEG_TEXT], "\tEND_DO\n", 8 );

        if ( tk != While ) mccfail( "missing while in do-while statement" );

        ntok();
        if ( tk != '(' ) mccfail( "missing opening parenthasis in switch statement" );
        ntok();

        void * erbp = sbrk(0);

        struct mccnode * root = expr( curnsp, ')' );

        if ( tk != ')' ) mccfail( "missing closing parenthasis in switch statement" );
        ntok();
        if ( tk != ';' ) mccfail( "missing final semicolon in do-while statement" );
        ntok();

        emit(curnsp, root);
        brk(erbp);
    }
    else
    {
        void * erbp = sbrk(0); // Expression RollBack Point

        struct mccnode * root = expr( curnsp, ';' );

        if ( tk != ';' ) mccfail( "Expected semicolon after expression" );
        ntok();

        emit(curnsp, root);

        brk(erbp); // Free memory allocated by the expression
    }
}
