#include "dgnmcc.h"

unsigned int flags; // Store misc booleans
unsigned int curfno; // Current file number
unsigned int entrypos; // Starting address offset within the text segment
unsigned int stksize; // Additional stack size

struct symbol * syms, ** symsEnd; // Symbol table
unsigned int sympos = 0, symmax = 0; // Number of symbols in the table

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
