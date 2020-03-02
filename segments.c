// Memory segments
struct segment text = {0, NULL, NULL, SYM_TXT, SYM_GTXT};
struct segment data = {0, NULL, NULL, SYM_DAT, SYM_GDAT};
struct segment  bss = {0, NULL, NULL, SYM_BSS, SYM_GBSS};
// Segment currently being worked on
struct segment * curseg;

// Set a word in a segment, if type is not SYM_ABS/SYM_GABS then val is a symbol table index
void segset( struct segment * seg, unsigned int addr, unsigned char type, unsigned int val )
{
    while ( addr > PAGESIZE )
    {
        // Allocate another block
        if ( blk->next == NULL )
        {
            blk->next = sbrk( PAGESIZE + 1 );

            // Out of memory
            if ( blk->next < 0 ) exit(1);

            blk->next->next = NULL; // Terminate new final block
        }

        blk = blk->next;
        addr -= PAGESIZE;
    }

    blk->data[addr] = val;
}

unsigned int segget( struct segment * seg, unsigned int addr )
{
    while ( addr > PAGESIZE )
    {
        // Beyond end of allocated memory
        if ( blk->next == NULL )
        {
            exit(1);
        }

        blk = blk->next;
        addr -= PAGESIZE;
    }

    return blk->data[addr];
}
