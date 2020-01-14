#include "dgnasm.h"

char * skipWhite( char * str )
{
    for ( ; *str == ' ' || *str == '\t'; str++ );
    return str;
}

int dgnasmcmp( const char * str1, const char * str2, dgnasm * state )
{
    if ( state->caseSense )
    {
        return strcmp( str1, str2 );
    }
    else
    {
        return strcasecmp( str1, str2 );
    }
}

void xlog( int level, dgnasm * state, const char * format, ... )
{
    // Hide messages that are too verbose
    if ( level > state->verbosity ) return;

    printf( "[%s, line %d] ", state->curFile, state->curLine);

    // Print message severity
    switch ( level )
    {
        case 0:
            printf( "Info: " );
            break;
        case 1:
            printf( "Warning: " );
            break;
        case 2:
            printf( "Syntax Error: " );
            break;
        case 3:
            printf( "Assembly Error: " );
            break;
        case 4:
            printf( "Debug: " );
            break;
    }

    // Print logged message
    va_list args;
    va_start( args, format );
    vprintf( format, args );
    va_end( args );
}
