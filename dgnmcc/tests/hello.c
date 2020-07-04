extern void tty_putc( char output );

void printf( char * str )
{
    while ( *str ) tty_putc( *str++ );
}

int main( int argc, char ** argv )
{
    printf( "Output test value: " );

    int test;

    test = 4;

    if ( 4 == test )
    {
        int varA;

        test++;
        tty_putc( test + 48 );
        tty_putc( '\r' );
        tty_putc( '\n' );
    }
    else
    {
        int varB;

        printf( "WTF!\r\n" );
    }
}
