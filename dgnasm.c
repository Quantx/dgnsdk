#include "dgnasm.h"

unsigned int flags = 0; // Store misc booleans
unsigned char curfno = 1; // Current file number

#include "symbols.c"
#include "segments.c"
#include "tokenizer.c"
#include "assembler.c"

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
    while ( curfno < argc )
    {
        curseg = &text; // Reset current segment
        assemble( argv[curfno - 1], 0 );
        curfno++;
    }

    // *** Reset for next pass ***
    // Set max and clear pos
    text.max = text.pos; text.pos = 0;
    data.max = data.pos; data.pos = 0;
     bss.max =  bss.pos;  bss.pos = 0;

    // Did zero page overflow
    if ( zero.pos > (zero.max = 0xFF) ) exit(1);
    zero.pos = 0;

    // Reset current file number
    curfno = 1;

    // *** Run second pass for each file ***
    while ( curfno < argc )
    {
        curseg = &text; // Reset current segment
        assemble( argv[curfno - 1], 1 );
        curfno++;
    }

    return 0;
}
