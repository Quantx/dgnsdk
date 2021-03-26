struct test {
    int a;
    struct a {
        int d;
        int f;
    } b;
    int c;
    union {
        int d;
        int e;
    };
    int f;
};

int main(int argc, char ** argv )
{
    struct test {
        long b;
        int h;
    } mytest;

    int b;

    b = mytest.h;
}

