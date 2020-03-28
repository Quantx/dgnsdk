// Memory segments
struct segment text = {NULL, 0, 0, NULL, 0, 0, SYM_TEXT };
struct segment data = {NULL, 0, 0, NULL, 0, 0, SYM_DATA };
struct segment  bss = {NULL, 0, 0, NULL, 0, 0, SYM_BSS  };
struct segment zero = {NULL, 0, 0, NULL, 0, 0, SYM_ZERO };
// Segment currently being worked on
struct segment * curseg;

// Set a word in a segment
void segset( struct segment * seg, unsigned int reloc, unsigned int val )
{
    struct relocate * curRloc = NULL;

    // Can't output to a BSS segment
    if ( seg->sym == SYM_BSS ) asmfail( "tried to output to BSS segment" );

    // Not on relocation pass yet
    if ( ~flags & FLG_RLOC ) return;

    // Compute relocation address and increment
    if ( reloc ) curRloc = seg->rloc + seg->rlocPos++;
//    {
//        write( 1, "RELOC: ", 7 );
//        octwrite( 1, reloc );
//        write( 1, "\r\n", 2 );
//    }

    // Not on data pass yet
    if ( ~flags & FLG_DATA ) return;

    // Beyond end of memory
    if ( seg->dataPos > seg->dataSize ) asmfail( "tried to output beyond end of segment's data" );
    if ( seg->rlocPos > seg->rlocSize ) asmfail( "tried to output beyond end of segment's relocation info" );

    // Store relocation bits if needed
    if ( reloc )
    {
        // Add symbol offsets
        if      ( (reloc >> 1 & SYM_MASK) == SYM_TEXT ) val +=  stksize ? stksize << 10 : 256;
        else if ( (reloc >> 1 & SYM_MASK) == SYM_DATA ) val += (stksize ? stksize << 10 : 256) + text.dataSize;
        else if ( (reloc >> 1 & SYM_MASK) == SYM_BSS  ) val += (stksize ? stksize << 10 : 256) + text.dataSize + data.dataSize;

        curRloc->head = reloc;
        curRloc->addr = seg->dataPos;
    }

    seg->data[seg->dataPos] = val;
}
