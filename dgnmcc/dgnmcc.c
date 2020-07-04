#include "dgnmcc.h"

const char * sepname = "SYMDELIM";
unsigned int flags; // Store misc booleans
unsigned int curfno; // Current file number
unsigned int entrypos; // Starting address offset within the text segment
unsigned int stksize; // Additional stack size

struct symbol * syms; // Symbol table
struct symbol ** symsEnd;
unsigned int sympos = ASM_SIZE; // Number of symbols in the table

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
