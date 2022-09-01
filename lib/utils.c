#include "unistd.h"

// Output an octal number
void octwrite( int16_t nfd, unsigned int32_t val )
{
    write( nfd, "0", 1 );

    if ( !val ) return;

    int8_t tmpbuf[11];
    int16_t tmppos = 11;

    while ( val )
    {
        tmpbuf[--tmppos] = (val & 7) + '0';
        val >>= 3;
    }

    write( nfd, tmpbuf + tmppos, 11 - tmppos );
}

// Output decimal number
void decwrite( int16_t nfd, unsigned int16_t val )
{
    if ( !val )
    {
        write( nfd, "0", 1 );
        return;
    }

    int8_t tmpbuf[6];
    int16_t tmppos = 6;

    while ( val )
    {
        tmpbuf[--tmppos] = val % 10 + '0';
        val /= 10;
    }

    write( nfd, tmpbuf + tmppos, 6 - tmppos );
}

// Output msg and error
void die( int8_t * msg )
{
    int16_t i = 0;
    while ( msg[i] ) i++;

    write( 2, msg, i );
    write( 2, "\n", 1 );
    exit(1);
}

// Output the current token followed by a message
unsigned int32_t ctn; // Current token
void fail( int8_t * msg )
{
    octwrite( 2, ctn );
    write( 2, " ", 1 );
    die( msg );
}

/*
// We need to do this to fix ASLR with sbrk
#ifdef LINUX_COMPAT
int8_t heap_data[HEAP_SIZE];
void * heap = heap_data;
unsigned int16_t brk_ptr;
unsigned int16_t max_brk;

void * sbrk( int size )
{
    int8_t * ptr = heap + brk_ptr;

    brk_ptr += size;
    if ( brk_ptr > max_brk ) max_brk = brk_ptr;
    if ( brk_ptr > HEAP_SIZE ) return SBRKFAIL;

    return ptr;
}

int brk( void * ptr )
{
    if ( ptr < heap || ptr >= heap + brk_ptr ) return -1;

    brk_ptr = (unsigned int16_t)(ptr - heap);
    return 0;
}
#endif
*/
