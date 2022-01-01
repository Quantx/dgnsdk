int a, b, c;

int myfunc()
{
    return a + b + c;
}

int myfuncargs( int a, int b, int c )
{
    return a + b + c;
}

int main ( int argc, char ** argv )
{
    myfunc();
    myfuncargs( 1, 2, 3 );
}
