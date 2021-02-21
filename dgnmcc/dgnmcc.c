#include "dgnmcc.h"

unsigned int16_t flags; // Store misc booleans
unsigned int16_t curfno; // Current file number
unsigned int16_t stksize; // Additional stack size

struct mccnsp glbnsp = { NULL, 0, 0, CPL_BLOCK, NULL, &glbnsp.symtbl, NULL, &glbnsp.nsptbl };

// Output an octal number
void octwrite( int16_t nfd, unsigned int16_t val )
{
    write( nfd, "0", 1 );

    if ( !val ) return;

    int8_t tmpbuf[6];
    int16_t tmppos = 6;

    while ( val )
    {
        tmpbuf[--tmppos] = (val & 7) + '0';
        val >>= 3;
    }

    write( nfd, tmpbuf + tmppos, 6 - tmppos );
}

#include "segments.c"
#include "tokenizer.c"
#include "symbols.c"
#include "types.c"
#include "expression.c"
#include "statement.c"
#include "declaration.c"
#ifdef DEBUG
#include "debug.c"
#endif

void mccfail( int8_t * msg )
{
    int16_t i = 0;
    while ( msg[i] ) i++;

    if ( ln )
    {
        octwrite( 2, ln );
        write( 2, ":", 1 );
    }

    write( 2, msg, i );
    write( 2, "\r\n", 2 );
    exit(1);
}

void compile( int8_t * fname )
{
    // Prime tokenizer
    sfd = open( fp = fname, 0 );
    if ( sfd < 0 ) mccfail( "Unable to open file for compilation" );

    ln = 1;
    readline();

    ntok();
    while ( tk )
    {
        define(&glbnsp);
    }
}

int16_t main( int16_t argc, int8_t ** argv )
{
    int8_t * progname = *argv++; argc--;

#if DEBUG
    // Sanity check to make sure we can store constants all in a char
    if ( Arrow > 0xFF ) mccfail("too many constants!");
#endif

    // Process all argument options
    while ( argc && **argv == '-' )
    {
        (*argv)++;

        argv++; argc--;
    }

    // Prefrom a first pass on each file
    while ( curfno < argc )
    {
        compile( argv[curfno] );
        curfno++;
    }
}
