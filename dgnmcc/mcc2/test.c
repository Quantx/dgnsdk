
struct a {
    int b;
};

struct a f(void)
{
    return (struct a){0};
}

int main()
{
    int c = f().b;
}
