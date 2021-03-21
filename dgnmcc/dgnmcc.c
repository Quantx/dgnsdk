#include "dgnmcc.h"

unsigned int16_t flags; // Store misc booleans
unsigned int16_t curfno; // Current file number
unsigned int16_t stksize; // Additional stack size
int16_t segs[5]; // Text, Constant, Zero, Data, Bss

struct mccnsp glbnsp = { NULL, 0, CPL_BLOCK, 0, NULL, &glbnsp.symtbl, NULL, &glbnsp.nsptbl };

// Output an octal number
void octwrite( int16_t nfd, unsigned int32_t val )
{
    write( nfd, "0", 1 );

    if ( !val ) return;

    int8_t tmpbuf[11];
    int16_t tmppos = 11;

    while ( val )
    {
        tmpbuf[--tmppos] = (val & 7) + '0';
        val >>= 3;
    }

    write( nfd, tmpbuf + tmppos, 11 - tmppos );
}

void decwrite( int16_t nfd, unsigned int16_t val )
{
    if ( !val ) return (void)write( nfd, "0", 1 );

    int8_t tmpbuf[6];
    int16_t tmppos = 6;

    while ( val )
    {
        tmpbuf[--tmppos] = val % 10 + '0';
        val /= 10;
    }

    write( nfd, tmpbuf + tmppos, 6 - tmppos );
}

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
        decwrite( 2, ln );
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
    // Sanity check to make sure we can store all constants in a char
    if ( Arrow > 0xFF ) mccfail("too many constants!");

    int16_t tnc = (sizeof(tokenNames)/sizeof(*tokenNames));
    int16_t tkc = (Arrow - Void) + 2;
    if ( tnc != tkc )
    {
        decwrite(2, tnc);
        write(2, "\n", 1);
        decwrite(2, tkc);
        write(2, "\n", 1);
        mccfail("debug token count missmatch");
    }
#endif

    // Open output files
    if ( (segs[SEG_ZERO] = creat( "zero.seg", 0666 )) < 0 ) mccfail("failed to create zero segment");
    if ( (segs[SEG_TEXT] = creat( "text.seg", 0666 )) < 0 ) mccfail("failed to create text segment");
    if ( (segs[SEG_CNST] = creat( "cnst.seg", 0666 )) < 0 ) mccfail("failed to create cnst segment");
    if ( (segs[SEG_DATA] = creat( "data.seg", 0666 )) < 0 ) mccfail("failed to create data segment");
    if ( (segs[SEG_BSS ] = creat( "bss.seg",  0666 )) < 0 ) mccfail("failed to create bss  segment");

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

    return 0;
}
