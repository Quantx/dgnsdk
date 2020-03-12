// Memory segments
struct segment text = {0, 0, NULL, NULL, SYM_TEXT };
struct segment data = {0, 0, NULL, NULL, SYM_DATA };
struct segment  bss = {0, 0, NULL, NULL, SYM_BSS  };
struct segment zero = {0, 0, NULL, NULL, SYM_ZERO };
// Segment currently being worked on
struct segment * curseg;

// Set a word in a segment
void segset( struct segment * seg, unsigned char redir, unsigned int val )
{
    // Still on pass 1
    if ( ~flags & FLG_PASS ) return;

    unsigned int addr = seg->pos;

    // Beyond end of memory
    if ( addr > seg->max ) asmfail( "tried to output beyond end of segment" );
    if ( seg->sym == SYM_BSS ) asmfail( "tried to output to BSS segment" );

    struct memblock * blk = seg->head; // Segment data
    struct memblock * rdr = seg->rdir; // Redirection

    while ( addr > PAGESIZE )
    {
        blk = blk->next;
        rdr = rdr->next;

        addr -= PAGESIZE;
    }

    if ( blk == NULL ) asmfail( "block was null!" );

    blk->data[addr] = val;
    rdr->data[addr] = redir;
}

void segalloc( struct segment * seg )
{
    unsigned int size = seg->max;
    struct memblock ** blk = &seg->head; // Segment data
    struct memblock ** rdr = &seg->rdir; // Redirection

    while ( size > 0 )
    {
        // Allocate another block
        if ( *blk == NULL )
        {
            *blk = (struct memblock *) sbrk( sizeof(struct memblock) );
            if ( *blk < 0 ) asmfail("out of memory");
            (*blk)->next = NULL; // Terminate new segment

            *rdr = (struct memblock *) sbrk( sizeof(struct memblock) );
            if ( *rdr < 0 ) asmfail("out of memory");
            (*rdr)->next = NULL; // Terminate new segment
        }

        blk = &(*blk)->next;
        rdr = &(*rdr)->next;

        if ( size >= PAGESIZE ) size -= PAGESIZE;
        else size = 0;
    }
}
