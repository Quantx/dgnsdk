#include "dgnasm.h"

int computeArgs( char * str, char ** argv )
{
    if ( str == NULL ) return 0;

    int i;
    char * curArg = strtok( str, ", \t" );

    for ( i = 0; i < MAX_ARGS && curArg != NULL; i++ )
    {
        argv[i] = curArg;
        curArg = strtok( NULL, ", \t" );
    }

    // Return number of args
    return i;
}

int checkArgs( int argc, int minArg, int maxArg, dgnasm * state )
{
    if ( minArg == maxArg && argc != minArg )
    {
        xlog( DGNASM_LOG_SYTX, state, "Incorrect number of arguments '%d', expected %d\n", argc, minArg );
    }
    else if ( argc < minArg )
    {
        xlog( DGNASM_LOG_SYTX, state, "Too few arguments %d, expected %d to %d\n", argc, minArg, maxArg );
    }
    else if ( argc > maxArg )
    {
        xlog( DGNASM_LOG_SYTX, state, "Too many arguments %d, expected %d to %d\n", argc, minArg, maxArg );
    }
    else
    {
        return 1;
    }

    return 0;
}

int convertNumber( char * str, unsigned short * val, int halfVal, unsigned short maxVal, dgnasm * state )
{
    int len = strlen(str);

    // Account for edge case
    if ( len == 0 )
    {
        xlog( DGNASM_LOG_DBUG, state, "Convert number was passed an empty string\n" );
        return 0;
    }

    // Use octal by default
    int base = 8;

    char * valPos = str;

    // Is this a character literal?
    if ( str[0] == '\'' )
    {
        // No convert
        base = -1;

        // One character literal
        if ( str[1] != '\0' && str[2] == '\'' )
        {
            *val = (short)str[1];
        }
        // Two character literal
        else if ( str[1] != '\0' && str[2] != '\0' && str[3] == '\'' )
        {
            *val = (short)(((short)str[1]) << 8) | str[2];
        }
        // Invalid
        else
        {
            xlog( DGNASM_LOG_SYTX, state, "Invalid character literal: >%s<\n", str );
            return 0;
        }
    }
    // Check if this is a hex constant
    else if ( str[0] == '$' )
    {
        valPos++;
        base = 16;
    }
    else if ( str[len - 1] == 'h' )
    {
        str[len - 1] = '\0';
        base = 16;
    }
    else if ( str[0] == '0' && str[1] == 'x' )
    {
        valPos += 2;
        base = 16;
    }
    // Check if binary constant
    else if ( str[0] == '%' )
    {
        valPos++;
        base = 2;
    }
    else if ( str[len - 1] == 'b' )
    {
        str[len - 1] = '\0';
        base = 2;
    }
    else if ( str[0] == '0' && str[1] == 'b' )
    {
        valPos += 2;
        base = 2;
    }
    // Check if decimal constant
    else if ( str[0] == '*' )
    {
        valPos++;
        base = 10;
    }
    else if ( str[len - 1] == 'd' )
    {
        str[len - 1] = '\0';
        base = 10;
    }
    else if ( str[0] == '0' && str[1] == 'd' )
    {
        valPos += 2;
        base = 10;
    }

    char * end = '\0';

    if ( base >= 0 )
    {
        *val = (unsigned short)strtol( valPos, &end, base );
    }

    // We only want 1 byte, null the upper half
    if ( halfVal ) *val &= 0xFF;

    if ( *end != '\0' )
    {
        xlog( DGNASM_LOG_SYTX, state, "Invalid character '%c' in number literal '%s'\n", *end, str );
        return 0;
    }
    else if ( *val > maxVal )
    {
        xlog( DGNASM_LOG_SYTX, state, "Number literal '%s' %dd excedes maximum value of %dd\n", str, *val, maxVal );
        return 0;
    }

    return 1;
}

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

int dgnasmncmp( const char * str1, const char * str2, int len, dgnasm * state )
{
    if ( state->caseSense )
    {
        return strncmp( str1, str2, len );
    }
    else
    {
        return strncasecmp( str1, str2, len );
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
