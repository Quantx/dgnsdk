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
        struct mccnode * t = expr( curnsp );

        dumpTree(t);

        if ( tk != ';' ) mccfail( "Expected ; after expression" );
        ntok();
    }
}
