int a, b, c, d;

//int (*test)(int a, int b, int c);
int g(...);

int main(int argc, char ** argv )
{
//    a = b + (*(test + 5))(c + 4, d - 5, d + c, 5, 7);
    a = g();
}
