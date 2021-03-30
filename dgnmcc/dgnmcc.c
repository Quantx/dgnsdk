#include "dgnmcc.h"

unsigned int16_t flags; // Store misc booleans
unsigned int16_t curfno; // Current file number
unsigned int16_t stksize; // Additional stack size
int16_t segs[5]; // Text, Constant, Zero, Data, Bss

struct mccnsp glbnsp = { NULL, 0, CPL_BLOCK | CPL_DEFN, 0, 0, NULL, &glbnsp.symtbl, NULL, &glbnsp.nsptbl };

#include "../lib/utils.c"

#include "tokenizer.c"
#include "symbols.c"
#include "types.c"
#include "expression.c"
#include "statement.c"
#include "declaration.c"
#ifdef DEBUG
#include "debug.c"
#endif

int16_t main( int16_t argc, int8_t ** argv )
{
    int8_t * progname = *argv++; argc--;

#if DEBUG
    // Sanity check to make sure we can store all constants in a char
    if ( FnCall > 0xFF ) mccfail("too many constants!");


    int16_t tnc = (sizeof(tokenNames)/sizeof(*tokenNames));
    int16_t tkc = (FnCall - Void) + 1;
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
        // Prime tokenizer
        sfd = open( fp = argv[curfno], 0 );
        if ( sfd < 0 ) mccfail( "Unable to open file for compilation" );

        ln = 0;
        readline();

        ntok();
        while ( tk )
        {
            define(&glbnsp);
        }

        curfno++;
    }

#ifdef DEBUG
//    dumpGlbnsp("glbnsp.txt");
#endif

    return 0;
}
