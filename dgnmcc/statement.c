void statement( struct mccnsp * curnsp )
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
        while ( tk != '}' ) statement( cbnsp );
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
    else
    {
        void * erbp = sbrk(0); // Expression RollBack Point

        expr( curnsp, 0 ); // No stop token

        if ( tk != ';' ) mccfail( "Expected ; after expression" );
        ntok();

        brk(erbp); // Free memory allocated by the expression
    }
}
