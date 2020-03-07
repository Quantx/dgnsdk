// Memory segments
struct segment text = {0, 0, NULL, NULL, SYM_TEXT };
struct segment data = {0, 0, NULL, NULL, SYM_DATA };
struct segment  bss = {0, 0, NULL, NULL, SYM_BSS  };
struct segment zero = {0, 0, NULL, NULL, SYM_ZERO };
// Segment currently being worked on
struct segment * curseg;

// Set a word in a segment, if type is not SYM_ABS/SYM_GABS then val is a symbol table index
void segset( struct segment * seg, unsigned int addr, unsigned char type, unsigned int val )
{
    // Beyond end of memory
    if ( addr > seg->max ) exit(1);

    struct memblock * blk = seg->head; // Segment data
    struct memblock * rdr = seg->rdir; // Redirection

    while ( addr > PAGESIZE )
    {
        // Allocate another block
        if ( blk->next == NULL )
        {
            blk->next = sbrk( PAGESIZE + 1 );
            if ( blk->next < 0 ) exit(1); // Out of memory
            blk->next->next = NULL; // Terminate new segment

            rdr->next = sbrk( PAGESIZE + 1 );
            if ( rdr->next < 0 ) exit(1); // Out of memory
            rdr->next->next = NULL; // Terminate new segment
        }

        blk = blk->next;
        rdr = rdr->next;

        addr -= PAGESIZE;
    }

    if ( type == SYM_DEF ) exit(1); // Undefined local symbol

    blk->data[addr] = val;
    // TODO redir bits
}

unsigned int segget( struct segment * seg, unsigned int addr )
{
    return 0;
}
