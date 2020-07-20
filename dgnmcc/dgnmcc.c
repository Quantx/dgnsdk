#include "dgnmcc.h"

unsigned int flags; // Store misc booleans
unsigned int curfno; // Current file number
unsigned int entrypos; // Starting address offset within the text segment
unsigned int stksize; // Additional stack size

struct mccsym * symtbl;
unsigned int sympos = 0, symmax = 0; // Number of symbols in the table

void mccfail(char * msg)
{
    int i = 0;
    while ( msg[i] ) i++;

    write( 2, msg, i );
    exit(1);
}

#include "segments.c"
#include "tokenizer.c"
#include "compiler.c"

int main( char ** argv, int argc )
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
        curfno++;
    }
}
