#include "dgnmcc.h"

unsigned int flags; // Store misc booleans
unsigned int curfno; // Current file number
unsigned int stksize; // Additional stack size

struct mccnsp glbnsp = { NULL, 0, 0, CPL_BLOC, NULL, &glbnsp.symtbl, NULL, &glbnsp.nsptbl };

// Output an octal number
void octwrite( int nfd, unsigned int val )
{
    write( nfd, "0", 1 );

    if ( !val ) return;

    char tmpbuf[6];
    int tmppos = 6;

    while ( val )
    {
        tmpbuf[--tmppos] = (val & 7) + '0';
        val >>= 3;
    }

    write( nfd, tmpbuf + tmppos, 6 - tmppos );
}

void mccfail( char * msg )
{
    int i = 0;
    while ( msg[i] ) i++;

    write( 2, msg, i );
    exit(1);
}

#include "segments.c"
#include "tokenizer.c"
#include "symbols.c"
#include "expression.c"
#include "compiler.c"

int main( int argc, char ** argv )
{
    char * progname = *argv++; argc--;

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
