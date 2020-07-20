#include "dgnasm.h"
#include <errno.h>

unsigned int flags; // Store misc booleans
unsigned int curfno; // Current file number
unsigned int entrypos; // Starting address offset within the text segment
unsigned int stksize; // Additional stack size
unsigned int copybuf[CBUF_LEN]; // Used to transfer data from one file to another

#include "symbols.c"

// Output an octal number
void octwrite( int nfd, unsigned int val )
{
    write( nfd, "0", 1 );

    if ( !val ) return;

    char tmpbuf[6];
    int tmppos = 6;

    while ( val )
    {
        tmpbuf[--tmppos] = (val & 7) + '0';
        val >>= 3;
    }

    write( nfd, tmpbuf + tmppos, 6 - tmppos );
}

// Output a symbol from the table
void symwrite( int nfd, struct asmsym * cursym )
{
    int k = 0;
    while ( cursym->name[k++] );
    write( nfd, "NAME: ", 6 );
    write( nfd, cursym->name, k );

    write( nfd, " | ", 3 );
    octwrite( nfd, cursym->len );

    write( nfd, "\r\n", 2 );


    write( nfd, "TYPE: ", 6);
    octwrite( nfd, cursym->type );
    write( nfd, "\r\n", 2 );

    write( nfd, "VAL: ", 5 );
    octwrite( nfd, cursym->val );
    write( nfd, "\r\n", 2 );
}

#include "help.c"
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

    // Output curpos indicator
    i = 0;
    while ( i < p - lp )
    {
        write( 2, pp[i++] == '\t' ? "\t" : " ", 1 );
    }

    write( 2, "^\r\n", 3 );

    exit(1); // Quit program
}

int main( int argc, char ** argv )
{
    int i;
    struct asmsym * cursym;

    // Drop first argument (program name)
    char * progname = *argv;
    argc--; argv++;

    // Set all flags
    while ( argc && **argv == '-' )
    {
        (*argv)++;

        // All symbols are global
        if      ( **argv == 'g' ) flags |= FLG_GLOB;
        // Stack size follows
        else if ( **argv == 't' )
        {
            argc--; argv++;
            while ( **argv >= '0' && **argv <= '9' ) stksize = stksize * 10 + *(*argv)++ - '0';

            if ( stksize > 32 ) asmfail( "Number of stack pages specified exceeds maximum of 32" );
        }
        // Output mode
        else if ( **argv == 'm' )
        {
            (*argv)++;
            if      ( **argv == 'h' ) flags |= FLG_SMH;
            else if ( **argv == 'a' ) flags |= FLG_SMHA;
            else if ( **argv == 'v' ) flags |= FLG_TERM;
        }

        argc--; argv++;
    }

    if ( !argc ) showhelp( progname );

    write( 1, " *** Loading symbols ***\r\n", 26 );

    // *** Setup Assembler Defined Symbols ***
    cursym = symint; i = 0;
    while ( cursym )
    {
        cursym->name = symstrs[i];

        int k = 0;
        while ( symstrs[i][k] ) k++;
        cursym->len = k;

        if ( cursym->next == NULL ) symtail = symtbl = &cursym->next;
        cursym = cursym->next; i++;
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

        // Add file seperator symbol
        struct asmsym * sepsym = sbrk( sizeof(struct asmsym) );
        if ( sepsym == SBRKFAIL ) asmfail( "Could not allocate room for file seperator symbol" );

        *symtail = sepsym;
        symtail = &sepsym->next;

        sepsym->name = NULL;
        sepsym->len = i;
        sepsym->type = SYM_FILE;
        sepsym->val = text.data.pos;
        sepsym->next = NULL;

        assemble( argv[curfno] );

        curfno++;
    }

    // Make sure there are no undefined local labels
    cursym = *symtbl;
    while ( cursym != NULL )
    {
        if ( (cursym->type & SYM_MASK) == SYM_DEF && ~cursym->type & SYM_GLOB )
        {
            symwrite( 2, cursym );
            asmfail("found undefined local label");
        }
        cursym = cursym->next;
    }

    // Check for overflow
    if ( zero.data.pos > 0xFF ) asmfail("zero page overflow");

    // Create temp files
    text.data.fd = creat( "/tmp/text-data", 0666 );
    if ( text.data.fd < 0 ) asmfail("failed to create /tmp/text-data");
    data.data.fd = creat( "/tmp/data-data", 0666 );
    if ( data.data.fd < 0 ) asmfail("failed to create /tmp/data-data");
    zero.data.fd = creat( "/tmp/zero-data", 0666 );
    if ( zero.data.fd < 0 ) asmfail("failed to create /tmp/zero-data");

    text.rloc.fd = creat( "/tmp/text-rloc", 0666 );
    if ( text.rloc.fd < 0 ) asmfail("failed to create /tmp/text-rloc");
    data.rloc.fd = creat( "/tmp/data-rloc", 0666 );
    if ( data.rloc.fd < 0 ) asmfail("failed to create /tmp/data-rloc");
    zero.rloc.fd = creat( "/tmp/zero-rloc", 0666 );
    if ( zero.rloc.fd < 0 ) asmfail("failed to create /tmp/zero-rloc");

    // *** Reset for next pass ***
    text.data.size = text.data.pos; text.data.pos = 0;
    text.rloc.size = text.rloc.pos; text.rloc.pos = 0;

    data.data.size = data.data.pos; data.data.pos = 0;
    data.rloc.size = data.rloc.pos; data.rloc.pos = 0;

     bss.data.size =  bss.data.pos;  bss.data.pos = 0;
     bss.rloc.size =  bss.rloc.pos;  bss.rloc.pos = 0;

    zero.data.size = zero.data.pos; zero.data.pos = 0;
    zero.rloc.size = zero.rloc.pos; zero.rloc.pos = 0;

    // Reset current file number
    curfno = 0;
    flags |= FLG_DATA;

    write( 1, " *** Starting second pass ***\r\n", 31 );

    // *** Run data pass for each file ***
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

    // Seek start of file for final output
    close( text.data.fd ); text.data.fd = open( "/tmp/text-data", 0 );
    close( data.data.fd ); data.data.fd = open( "/tmp/data-data", 0 );
    close( zero.data.fd ); zero.data.fd = open( "/tmp/zero-data", 0 );

    close( text.rloc.fd ); text.rloc.fd = open( "/tmp/text-rloc", 0 );
    close( data.rloc.fd ); data.rloc.fd = open( "/tmp/data-rloc", 0 );
    close( zero.rloc.fd ); zero.rloc.fd = open( "/tmp/zero-rloc", 0 );

    // *** Output final result ***

    write( 1, "*** Generating output file ***\r\n", 32 );

    // Open a.out for writing
    int ofd = creat( "a.out", 0755 );
    if ( ofd < 0 ) asmfail( "failed to open a.out" );

    if ( flags & FLG_SMH ) // SimH output
    {
        struct segment * seg = &zero;
        unsigned int org = 0;
        int header[3];

        // Output Zero, Text, and Data segments
        while ( seg )
        {
            unsigned int bs = 0;
            int bl;
            // Read input in 16 word increments
            while ( bl = read( seg->data.fd, copybuf, 16 * sizeof(int) ) / sizeof(int) )
            {
                if ( bl < 0 ) asmfail( "read fail" );

                header[0] = -bl; // Negative block length
                header[1] = org + bs; // Location to put block in memory

                header[2] = -header[0] - header[1]; // Compute checksum
                i = 0;
                while ( i < bl ) header[2] -= copybuf[i++];

                write( ofd, header, 6 ); // Output header
                write( ofd, copybuf, bl * sizeof(int) ); // Output block

                bs += bl;
            }

            if      ( seg == &zero ) { org += stksize ? stksize << 10 : 256; seg = &text; }
            else if ( seg == &text ) { org += text.data.size; seg = &data; }
            else seg = NULL;
        }

        org += data.data.size;
        i = 0;

        // Output small BSS segment
        if ( bss.data.size <= 16 )
        {
            if ( bss.data.size )
            {
                header[0] = -bss.data.size;
                header[1] = org;
                header[2] = bss.data.size - org;

                write( ofd, header, 6 );

                while ( *header < 0 )
                {
                    write( ofd, &i, 2 );
                    (*header)++;
                }
            }
        }
        else
        {
            // Prime SimH with a single 0 byte block
            header[0] = -1;
            header[1] = org;
            header[2] = 1 - org;

            write( ofd, header, 6 );
            write( ofd, &i, 2 );

            // Send SimH a repeat block to fill out the rest of BSS
            header[0] = -bss.data.size + 1;
            header[1] = org + 1;
            header[2] = -header[0] - header[2];

            write( ofd, header, 6 );
        }

        // Output start block
        header[0] = 1;
        header[1] = (stksize ? stksize << 10 : 256) + entrypos;
        header[2] = 0;

        // Should we enable auto-start
        if ( flags & FLG_SMHA ) header[1] |= 0x8000;

        write( ofd, header, 6 );
    }
    else if ( flags & FLG_TERM ) // Virtual console output
    {
        struct segment * seg = &zero;
        unsigned int org = 0;

        write( ofd, "K", 1 ); // Make sure the current cell is closed

        while ( seg )
        {
            if ( seg->data.size )
            {
                // Open the first cell
                octwrite( ofd, org );
                write( ofd, "/", 1 );

                while ( i = read( seg->data.fd, copybuf, CBUF_LEN * sizeof(int) ) / sizeof(int) )
                {
                    if ( i < 0 ) asmfail( "read error" );

                    int k = 0;
                    while ( k < i )
                    {
                        octwrite( ofd, copybuf[k] );
                        write( ofd, "\n", 1 );
                        k++;
                    }
                }

                write( ofd, "K", 1 ); // Close the last cell
            }

            if      ( seg == &zero ) { org += stksize ? stksize << 10 : 256; seg = &text; }
            else if ( seg == &text ) { org += text.data.size; seg = &data; }
            else seg = NULL;
        }

        // Output BSS segment
        if ( bss.data.size )
        {
            org += data.data.size;

            // Open first cell
            octwrite( ofd, org );
            write( ofd, "/", 1 );

            i = 0;
            while ( i < bss.data.size )
            {
                write( ofd, "0\n", 2 );
                i++;
            }

            write( ofd, "K", 1 ); // Close last cell
        }

        // Initialize the stack pointer
        if ( stksize ) write( ofd, "5A400\nK", 7 );
    }
    else // Binary executable output
    {
        i = 0;
        cursym = *symtbl;
        while ( cursym ) { cursym = cursym->next; i++; }

        // Output header
	struct exec header;
        header.magic = 0; // Magic number (program load method)
        header.stack = stksize; // Stack segment length
        header.zero = zero.data.size; // Zero segment length
        header.zrsize = zero.rloc.size; // Number of relocation entries
        header.text = text.data.size; // Text segment length
        header.trsize = text.rloc.size; // Text segment relocation length
        header.data = data.data.size; // Data segment length
        header.drsize = data.rloc.size; // Data segment relocation length
        header.bss = bss.data.size; // Bss segment length
        header.syms = i; // Number of symbols in table
        header.entry = (stksize ? stksize << 10 : 256) + entrypos; // Text segment entry offset

        write( ofd, &header, sizeof(struct exec) );

        // Output each segment's data
        while ( i = read( zero.data.fd, copybuf, CBUF_LEN * sizeof(int) ) ) write( ofd, copybuf, i );
        while ( i = read( text.data.fd, copybuf, CBUF_LEN * sizeof(int) ) ) write( ofd, copybuf, i );
        while ( i = read( data.data.fd, copybuf, CBUF_LEN * sizeof(int) ) ) write( ofd, copybuf, i );

        // Output symbol table
        unsigned int nameoff = 0;
        struct symbol outsym;
        cursym = *symtbl;
        while ( cursym )
        {
            // Build output symbol
            outsym.stroff = nameoff;
            outsym.type = cursym->type;
            outsym.val  = cursym->val;

            write( ofd, &outsym, sizeof(struct symbol) );

            // Compute offset (+1 for null terminate)
            nameoff += cursym->len + 1;

            cursym = cursym->next; i++;
        }

        // Output string table
        unsigned int k = 0;
        cursym = *symtbl;
        while ( cursym )
        {
            if ( (cursym->type & SYM_MASK) == SYM_FILE )
            {
                write( ofd, argv[k++], cursym->len + 1 );
            }
            else
            {
                write( ofd, cursym->name, cursym->len + 1 );
            }

            cursym = cursym->next; i++;
        }

        // Output each segment's data
        while ( i = read( zero.rloc.fd, copybuf, CBUF_LEN * sizeof(int) ) ) write( ofd, copybuf, i );
        while ( i = read( text.rloc.fd, copybuf, CBUF_LEN * sizeof(int) ) ) write( ofd, copybuf, i );
        while ( i = read( data.rloc.fd, copybuf, CBUF_LEN * sizeof(int) ) ) write( ofd, copybuf, i );
    }

    // Close work files
    close( zero.data.fd ); close( zero.rloc.fd );
    close( text.data.fd ); close( text.rloc.fd );
    close( data.data.fd ); close( data.rloc.fd );

    // Close a.out
    close( ofd );

    return 0;
}
