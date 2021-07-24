/*
int a[3][4] = {
    {1},
    {2}
};

long b = 0xFFFFFFFF;
*/

//char * t = "test\a\n";

//int ****((*(*a)(int a))[4])[2];

/*
int a = 1, b, c, *d = &c & 3, *e, g[4];

struct myst {
    int a;
} mysta, * mystb = &mysta;

int *z = &mysta.a;
*/

//int a[256];

struct test_a {
    struct test_b * ptr;
};

struct test_b {
    struct test_a * ptr;
};

struct test {
    int a;
};

int a, c, g[4], b = 0 ? c : 4;

int main( int argc, char ** argv )
{
    struct test myt;

    struct test {
        int b;
    };

//    struct test myt;

    a = myt.b;

    a = (c + g)[ (a++ + -g[2]) * ++b ] + 4, 7 + (char _)b;
}
