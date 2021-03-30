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
void die( int8_t * msg, unsigned int16_t ln, unsigned int16_t chr )
{
    int16_t i = 0;
    while ( msg[i] ) i++;

    decwrite( 2, ln  );
    write( 2, ":", 1 );
    decwrite( 2, chr );
    write( 2, ":", 1 );

    write( 2, msg, i );
    write( 2, "\n", 1 );
    exit(1);
}
