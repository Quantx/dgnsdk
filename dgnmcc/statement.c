void statement( struct mccnsp * curnsp )
{
    if ( tk == ';' ) ntok();
    else if ( tk == '{' )
    {
        ntok();

        // TODO SCOPE STUFF

//        while ( tk != '}' ) statement( curnsp->child );
        ntok();

        // TODO Purge locals
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
