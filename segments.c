// Memory segments
struct segment text = {0, 0, NULL, NULL, SYM_TEXT };
struct segment data = {0, 0, NULL, NULL, SYM_DATA };
struct segment  bss = {0, 0, NULL, NULL, SYM_BSS  };
struct segment zero = {0, 0, NULL, NULL, SYM_ZERO };
// Segment currently being worked on
struct segment * curseg;

// Set a word in a segment, if type is not SYM_ABS/SYM_GABS then val is a symbol table index
void segset( struct segment * seg, unsigned char type, unsigned int val )
{
    unsigned char bytemod = type & SYM_BYTE; // Get Byte Flag

    type &= ~SYM_BYTE; // Unset Byte Flag

    unsigned int addr = seg->pos;

    // Beyond end of memory
    if ( addr > seg->max ) exit(1);

    struct memblock * blk = seg->head; // Segment data
    struct memblock * rdr = seg->rdir; // Redirection

    while ( addr > PAGESIZE )
    {
        blk = blk->next;
        rdr = rdr->next;

        addr -= PAGESIZE;
    }

    if ( type == SYM_DEF ) exit(1); // Undefined local symbol

    blk->data[addr] = val;
    // TODO redir bits
}

void segalloc( struct segment * seg )
{
    unsigned int size = seg->max;
    struct memblock * blk = seg->head; // Segment data
    struct memblock * rdr = seg->rdir; // Redirection

    while ( size > PAGESIZE )
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

        size -= PAGESIZE;
    }
}
