// Memory segments
struct segment text = {0, 0, NULL, NULL, SYM_TEXT };
struct segment data = {0, 0, NULL, NULL, SYM_DATA };
struct segment  bss = {0, 0, NULL, NULL, SYM_BSS  };
struct segment zero = {0, 0, NULL, NULL, SYM_ZERO };
// Segment currently being worked on
struct segment * curseg;

// Set a word in a segment
void segset( struct segment * seg, unsigned char reloc, unsigned int val )
{
    // Still on pass 1
    if ( ~flags & FLG_PASS ) return;

    unsigned int addr = seg->pos;

    // Beyond end of memory
    if ( addr > seg->max ) asmfail( "tried to output beyond end of segment" );
    if ( seg->sym == SYM_BSS ) asmfail( "tried to output to BSS segment" );

    struct memblock * blk = seg->head; // Segment data
    struct memblock * rlc = seg->rloc; // Relocation

    while ( addr > PAGESIZE )
    {
        blk = blk->next;
        rlc = rlc->next;

        addr -= PAGESIZE;
    }

    if ( blk == NULL ) asmfail( "block was null!" );

    blk->data[addr] = val;
    rlc->data[addr] = reloc;
}

void segalloc( struct segment * seg )
{
    unsigned int size = seg->max;
    struct memblock ** blk = &seg->head; // Segment data
    struct memblock ** rlc = &seg->rloc; // Relocation

    while ( size > 0 )
    {
        // Allocate another block
        if ( *blk == NULL )
        {
            *blk = (struct memblock *) sbrk( sizeof(struct memblock) );
            if ( *blk < 0 ) asmfail("out of memory");
            (*blk)->next = NULL; // Terminate new segment

            *rlc = (struct memblock *) sbrk( sizeof(struct memblock) );
            if ( *rlc < 0 ) asmfail("out of memory");
            (*rlc)->next = NULL; // Terminate new segment
        }

        blk = &(*blk)->next;
        rlc = &(*rlc)->next;

        if ( size >= PAGESIZE ) size -= PAGESIZE;
        else size = 0;
    }
}

void blkwrite( int sfd, struct memblock * blk, unsigned int size )
{
    while ( size >= PAGESIZE )
    {
        write( sfd, blk->data, PAGESIZE << 1 );

        blk = blk->next;

        size -= PAGESIZE;
    }

    if ( size ) write( sfd, blk->data, size << 1 );
}
