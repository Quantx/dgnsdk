#include "dgnasm.h"

char * sepname = "SYMDELIM";
unsigned int flags = 0; // Store misc booleans
unsigned char curfno = 0; // Current file number
unsigned int entrypos = 0;

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
        while ( fp[i++] );
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
    while ( msg[i++] );
    write( 2, msg, i );
    write( 2, "\r\n", 2 );

    // Output current line
    i = 0;
    while ( lp[i++] );
    write( 2, lp, i );
//    write( 2, "\r\n", 2 );

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
    int i;

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

        i = 0;
        while( progname[i++] );
        write( 2, progname, i );

        write( 2, " [-] file1.s file2.s ...\r\n", 26 );

        exit(1);
    }

    write( 1, " *** Starting first pass ***\r\n", 30 );

    // *** Run first pass for each file ***
    while ( curfno < argc )
    {
        // Output the current file
        write( 1, "Labeling file: ", 15 );
        i = 0;
        while ( argv[curfno][i++] );
        write( 1, argv[curfno], i );
        write( 1, "\r\n", 2 );

        assemble( argv[curfno] );

        if ( curfno < argc - 1 )
        {
            // Add file seperator symbol
            struct symbol * sepsym = symtbl + sympos++;

            i = 0;
            while ( sepname[i] ) { sepsym->name[i] = sepname[i]; i++; }
            sepsym->type = SYM_FILE;
            sepsym->val = 0;
        }

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
    curfno = 0;
    flags |= FLG_PASS;

    // *** Run second pass for each file ***
    while ( curfno < argc )
    {
        // Output the current file
        write( 1, "Assembling file: ", 17 );
        i = 0;
        while ( argv[curfno][i++] );
        write( 1, argv[curfno], i );
        write( 1, "\r\n", 2 );

        assemble( argv[curfno] );
        curfno++;
    }

    // *** Output final result ***

    // Open a.out for writing
    int ofd = creat( "a.out", 0755 );

    if ( ofd < 0 ) asmfail("failed to open a.out");

    // Output header
    int header[8];
    header[0] = 0410;     // Magic number (program load method)
    header[1] = zero.max; // Zero segment length
    header[2] = text.max; // Text segment length
    header[3] = data.max; // Data segment length
    header[4] =  bss.max; // Bss  segment length
    header[5] = sympos - ASM_SIZE; // Symbol table length
    header[6] = entrypos; // Text segment entry offset
    header[7] = 0;        // Misc flags

    write( ofd, header, 16 );

    // Output each segment's data
    blkwrite( ofd, zero.head, zero.max );
    blkwrite( ofd, text.head, text.max );
    blkwrite( ofd, data.head, data.max );

    // Output symbol table
    write( ofd, symtbl + ASM_SIZE, header[5] * sizeof(struct symbol) );

    // Output relocation data
    blkwrite( ofd, zero.rloc, zero.max );
    blkwrite( ofd, text.rloc, text.max );
    blkwrite( ofd, data.rloc, data.max );

    // Close a.out
    close( ofd );

    return 0;
}

