// Current token
unsigned int32_t ctn;

// Token information
unsigned int8_t tk;
int16_t tkVal;
int32_t tkLong;
int8_t tkStr[256];

// Current string constant
unsigned int16_t csc;

void mccfail( int8_t * msg )
{
    octwrite( 2, ctn );
    write( 2, " ", 1 );
    die( msg );
}

#ifdef DEBUG_TOKEN
void debug_ntok()
#else
void ntok()
#endif
{
    ctn++;

    // Try to read the next token in
    if ( read( 0, &tk, 1 ) <= 0 ) { tk = EndOfFile; return; }

    switch ( tk )
    {
        case Named:
            if ( read( 0, &tkVal, 2 ) < 2 ) mccfail( "expected 16 bit value" );
            if ( read( 0, tkStr, tkVal ) < tkVal ) mccfail( "not enough characters in string");
            break;
        case String:

            if ( read( 0, &tkVal, 2 ) < 2 ) mccfail( "expected 16 bit value" );

            write( segs[SEG_CNST], "~SC", 3 );
            decwrite( segs[SEG_CNST], csc );
            write( segs[SEG_CNST], ": \"", 3 );

            int16_t i;
            for ( i = 0; i < tkVal; i++ )
            {
                int8_t sc;
                read( 0, &sc, 1 );
                write( segs[SEG_CNST], &sc, 1 );
            }

            write( segs[SEG_CNST], "\"\n", 2 );

            tkVal = csc++;
            break;
        case Number:
        case SmolNumber:
            if ( read( 0, &tkVal, 2 ) < 2 ) mccfail( "expected 16 bit value" );
            break;
        case LongNumber:
            if ( read( 0, &tkLong, 4 ) < 4 ) mccfail( "expected 32 bit value" );
            break;
    }
}

#ifdef DEBUG_TOKEN
void ntok()
{
    debug_ntok();

    write( 2, "Token '", 7 );
    writeToken( 2, tk );
    write( 2, "'", 1 );
/*
    if ( tk )
    {
        write( 1, ": ", 3 );
        write( 1, pp, p - pp );
    }
*/
    write( 2, "\r\n", 2 );
}
#endif
