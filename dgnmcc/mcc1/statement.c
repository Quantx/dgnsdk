void parenthesizedExpr( struct mccnsp * curnsp )
{
    if ( tk != Paren_L ) fail( "missing opening parenthasis in statement" );
    ntok();

    void * erbp = sbrk(0);

    struct mccnode * root = expr( curnsp, Paren_R );

    if ( tk != Paren_R ) fail( "missing closing parenthasis in statement" );

    emit(root);

    brk(erbp);

    ntok();
}

void statement( struct mccsym * func, struct mccnsp * curnsp, int sws )
{
    if ( tk == SemiColon )
    {
        emitStatement( Void, 0 ); // No-op statement
        ntok();
    }
    else if ( tk == Curly_L )
    {
        struct mccnsp * cbnsp = sbrk(sizeof(struct mccnsp CAST_NAME));
        if ( cbnsp == SBRKFAIL ) fail("unable to allocate room for child block");

        cbnsp->name = NULL;
        cbnsp->len = 0;

        cbnsp->type = CPL_BLOCK | CPL_DEFN;
        cbnsp->addr = curnsp->size; // Inherit offset
        cbnsp->size = 0;

        cbnsp->symtbl = NULL; cbnsp->symtail = &cbnsp->symtbl;
        cbnsp->nsptbl = NULL; cbnsp->nsptail = &cbnsp->nsptbl;

        cbnsp->parent = curnsp;
        cbnsp->next = NULL;

        // Add to parent namespace
        *curnsp->nsptail = cbnsp;
        curnsp->nsptail = &cbnsp->next;

        ntok();
        while ( tk != Curly_R ) statement( func, cbnsp, sws );
        ntok();

        // Unallocate current namespace from the stack
        emitStatement( Unallocate, cbnsp->size );

        // Drop child from parent namespace table
        struct mccnsp ** cnsp;
        int16_t i = 0;
        for ( cnsp = &curnsp->nsptbl; *cnsp && *cnsp != cbnsp; cnsp = &(*cnsp)->next )i++;

#if DEBUG
        if ( !(*cnsp) ) fail( "cnsp is null" );
#endif

        // Make sure namespace tail doesn't become detached
        if ( curnsp->nsptail == &(*cnsp)->next ) curnsp->nsptail = cnsp;

        *cnsp = (*cnsp)->next; // Drop current namespace

#if DEBUG
        if ( curnsp->nsptail != &curnsp->nsptbl ) fail("child block tail dropped");
#endif

        // Unallocate child block
        brk(cbnsp);
    }
    else if ( tk >= Void && tk <= Const )
    {
        define(curnsp);
    }
    else if ( tk == If )
    {
        emitStatement( If, 0 );

        ntok();
        parenthesizedExpr(curnsp);

        statement( func, curnsp, sws);

        if ( tk == Else )
        {
            emitStatement( Else, 0 );

            ntok();
            statement( func, curnsp, sws);
        }

        emitStatement( End, 0 );
    }
    else if ( tk == Switch )
    {
        emitStatement( Switch, 0 );

        ntok();
        parenthesizedExpr(curnsp);

        statement(func, curnsp, 1);

        emitStatement( End, 0 );
    }
    else if ( tk == Case )
    {
        if ( !sws ) fail("case statement outside of switch statement");

        ntok();

        void * erbp = sbrk(0);

        struct mccnode * root = expr( curnsp, Colon );

        if ( root->oper < Number || root->oper > LongNumber ) fail("need constant expression in case statement");

        if ( tk != Colon ) fail( "missing colon in case statement" );
        ntok();

        emitStatement( Case, root->val );

        brk(erbp);
    }
    else if ( tk == Default )
    {
        emitStatement( Default, 0 );

        if ( !sws ) fail("default statement outside of switch statement");

        ntok();
        if ( tk != Colon ) fail( "missing colon after default statement" );
        ntok();
    }
    else if ( tk == Break )
    {
        emitStatement( Break, 0 );

        ntok();
        if ( tk != SemiColon ) fail( "missing semicolon after break statement" );
        ntok();
    }
    else if ( tk == Continue )
    {
        emitStatement( Continue, 0 );

        ntok();
        if ( tk != SemiColon ) fail( "missing semicolon after continue statement" );
        ntok();
    }
    else if ( tk == Return )
    {
        ntok();

        struct mcctype * frt = typeClone(&func->type);
        struct mccsubtype * s;
        for ( s = frt->sub; s->sub; s = s->sub );
        s->ftype = NULL;

        if ( tk == SemiColon )
        {
            // Check if function expects void
            if (!isCompatible( frt, &type_void ) ) fail("function expects non-void return type");

            ntok();
            emitStatement( Return, 1 ); // NOTE: The 1 implies a VOID return type
            brk(frt);
            return;
        }

        emitStatement( Return, 0 );

        struct mccnode * root = expr( curnsp, SemiColon );

        if ( tk != SemiColon ) fail( "missing semicolon after return statement" );
        ntok();

        if ( !isCompatible( frt, root->type ) ) fail("incompatible return type");

        emit(root);

        brk(frt); // Also rolls back the expression
    }
    else if ( tk == For )
    {
        emitStatement( For, 0 );

        ntok();
        if ( tk != Paren_L ) fail( "missing opening parenthasis in for statement" );
        ntok();

        void * erbp;
        struct mccnode * root;

        // Init expression
        if ( tk != SemiColon )
        {
            erbp = sbrk(0);

            root = expr( curnsp, SemiColon );

            if ( tk != SemiColon ) fail( "missing first semicolon in for statement" );

            emit(root);
            brk(erbp);
        }
        else emitStatement( Void, 0 ); // No-op statement
        ntok();

        // Eval expression
        if ( tk != SemiColon )
        {
            erbp = sbrk(0);

            root = expr( curnsp, SemiColon );

            if ( tk != SemiColon ) fail( "missing second semicolon in for statement" );

            emit(root);
            brk(erbp);
        }
        else emitStatement( SmolNumber, 1 ); // Loop forever
        ntok();

        // Iterator expression
        if ( tk != Paren_R )
        {
            erbp = sbrk(0);

            root = expr( curnsp, Paren_R );

            if ( tk != Paren_R ) fail( "missing closing parenthasis in for statement" );

            emit(root);
            brk(erbp);
        }
        else emitStatement( Void, 0 ); // No-op statement
        ntok();

        // Statement

        statement(func, curnsp, sws);

        emitStatement( End, 0 );
    }
    else if ( tk == While )
    {
        emitStatement( While, 0 );

        ntok();
        parenthesizedExpr(curnsp);

        statement(func, curnsp, sws);

        emitStatement( End, 0 );
    }
    else if ( tk == Do )
    {
        emitStatement( Do, 0 );

        ntok();
        statement(func, curnsp, sws);

        if ( tk != While ) fail( "missing while in do-while statement" );        
        emitStatement( While, 1 );

        ntok();
        parenthesizedExpr(curnsp);

        if ( tk != SemiColon ) fail( "missing final semicolon in do-while statement" );
        ntok();
        
        emitStatement( End, 0 );
    }
    else
    {
        void * erbp = sbrk(0); // Expression RollBack Point

        struct mccnode * root = expr( curnsp, SemiColon );

        if ( tk != SemiColon ) fail( "Expected semicolon after expression" );

        emit(root);

        brk(erbp); // Free memory allocated by the expression

        ntok();
    }
}
