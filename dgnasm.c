#include "dgnasm.h"

unsigned int flags = 0; // Store misc booleans
unsigned char curfno = 1; // Current file number

// Output an octal number
void octwrite( int nfd, unsigned int val )
{
    char tmpbuf[6];
    int tmppos = 5;

    while ( val )
    {
        tmpbuf[tmppos--] = (val & 7) + '0';
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
    }

    // Output error message
    i = 0;
    while ( msg[i] ) i++;
    write( 2, msg, i );

    // Output newline and exit
    write( 2, "\r\n", 2 );
    exit(1);
}

int main( int argc, char ** argv )
{
    // Drop first argument (program name)
    argc--; argv++;

    // Force all undefined symbols to be globals
    if ( argc && **argv == '-' )
    {
        flags |= FLG_GLOB;
        argc--; argv++;
    }

    // *** Run first pass for each file ***
    while ( curfno <= argc )
    {
        assemble( argv[curfno - 1], 0 );
        curfno++;
    }

    // *** Reset for next pass ***
    // Set max and clear pos
    text.max = text.pos; text.pos = 0;
    data.max = data.pos; data.pos = 0;
     bss.max =  bss.pos;  bss.pos = 0;
    zero.max = zero.pos; zero.pos = 0;

    // Did zero page overflow?
    if ( zero.max > 0xFF ) asmfail("zero page overflow");

    // Allocate memory for each segment
    segalloc( &text );
    segalloc( &data );
    segalloc(  &bss );
    segalloc( &zero );

    // Reset current file number
    curfno = 1;

    // *** Run second pass for each file ***
    while ( curfno <= argc )
    {
        assemble( argv[curfno - 1], 1 );
        curfno++;
    }

    return 0;
}
