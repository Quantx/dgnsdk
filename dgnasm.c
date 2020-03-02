#include "dgnasm.h"

unsigned int flags = 0; // Store misc booleans

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

    int i = 0;
    // Assemble each file
    while ( i < argc )
    {
        curseg = &text; // Reset current segment
        pass1( argv[i] );

        i++;
    }

    return 0;
}
