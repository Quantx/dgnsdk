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
        expr( curnsp );

        if ( tk != ';' ) mccfail( "Expected ; after expression" );
        ntok();
    }
}
