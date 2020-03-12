#include "dgnasm.h"

unsigned int flags = 0; // Store misc booleans
unsigned char curfno = 1; // Current file number

// Output an octal number
void octwrite( int nfd, unsigned int val )
{
    if ( !val )
    {
        write( 1, "0", 1 );
        return;
    }

    char tmpbuf[6];
    int tmppos = 6;

    while ( val )
    {
        tmpbuf[--tmppos] = (val & 7) + '0';
        val >>= 3;
    }

    write( 1, tmpbuf + tmppos, 6 - tmppos );
}

#include "symbols.c"
#include "segments.c"
#include "tokenizer.c"
#include "assembler.c"

void asmfail( char * msg )
{
    int i = 0;

    if ( fp )
    {
        unsigned int tmppos;
        char tmpchr;

        // Output current file
        while ( fp[i] ) i++;
        write( 2, fp, i );
        write( 2, ":", 1 );

        if ( p )
        {
            // Output current line in octal
            octwrite( 2, curline );
            write( 2, ":", 1 );

            // Output current position in octal
            octwrite( 2, pp - lp );
            write( 2, ":", 1 );
        }

        // Close current file (good practice)
        close( fd );
    }

    // Output error message
    i = 0;
    while ( msg[i] ) i++;
    write( 2, msg, i );
    write( 2, "\r\n", 2 );

    // Output current line
    i = 0;
    while ( lp[i] ) i++;
    write( 2, lp, i );
    write( 2, "\r\n", 2 );

    // Output curpos indicator
    i = 0;
    while ( i < pp - lp )
    {
        write( 2, lp[i] == '\t' ? "\t" : " ", 1 );
        i++;
    }

    write( 2, "^\r\n", 3 );

    exit(1); // Quit program
}

int main( int argc, char ** argv )
{
    // Drop first argument (program name)
    char * progname = *argv;
    argc--; argv++;

    // Force all undefined symbols to be globals
    if ( argc && **argv == '-' )
    {
        flags |= FLG_GLOB;
        argc--; argv++;
    }

    if ( !argc )
    {
        write( 2, "usage: ", 7 );
        int i = 0;
        while( progname[i] ) i++;
        write( 2, progname, i );
        write( 2, " [-] file1.s file2.s ...\r\n", 26 );

        exit(1);
    }

    write( 1, " *** Starting first pass ***\r\n", 30 );

    // *** Run first pass for each file ***
    while ( curfno <= argc )
    {
        assemble( argv[curfno - 1] );
        curfno++;
    }

    write( 1, "Allocating required memory\r\n", 28 );

    // *** Reset for next pass ***
    // Set max and clear pos
    text.max = text.pos; text.pos = 0;
    data.max = data.pos; data.pos = 0;
     bss.max =  bss.pos;  bss.pos = 0;
    zero.max = zero.pos; zero.pos = 0;

    // Did zero page overflow?
    if ( zero.max > 0xFF ) asmfail("zero page overflow");

    // Allocate memory for each segment, except for BSS
    segalloc( &text );
    segalloc( &data );
    segalloc( &zero );


    write( 1, " *** Starting second pass ***\r\n", 31 );

    // Reset current file number
    curfno = 1;
    flags |= FLG_PASS;

    // *** Run second pass for each file ***
    while ( curfno <= argc )
    {
        assemble( argv[curfno - 1] );
        curfno++;
    }

    // *** Output final result ***

    // Open a.out for writing
    int ofd = open( "a.out", 1 );

    if ( ofd < 0 ) asmfail("failed to open a.out");

    

    // Close a.out
    close( ofd );

    return 0;
}
