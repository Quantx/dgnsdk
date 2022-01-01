struct mystruct {
    int a, b, c;
};

struct mystruct modify( struct mystruct a, long b, int c )
{
    a.b = b;
    a.c = a.a + c;

    return a;
}

int main( int argc, char ** argv )
{
    struct mystruct a;
    
    a.a = 1;
    a.b = 9999;
    a.c = -152;
    
    struct mystruct b = modify( a, 123, -34 );

    int c = -784;
    int d = 154;

    int e = modify( a, ++d, c++ ).b;

    int * f = &e;
}
