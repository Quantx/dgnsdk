#include "mcc2.h"

unsigned int16_t curfno;

#include "../../lib/utils.c"

#include "node.c"
#include "expression.c"
#include "statement.c"


int main( int argc, char ** argv )
{
    // Process all argument options
    while ( argc && **argv == '-' )
    {
        (*argv)++;

        argv++; argc--;
    }

    struct mccstmt * st;
    while ( st = node() )
    {
        statement(st);
        brk(st);
    }
}
