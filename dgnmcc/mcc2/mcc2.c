#include "mcc2.h"

unsigned int16_t curfno;

#include "../../lib/utils.c"

int16_t zerosize; // Current size of the zero page resident portion of the local stack
int16_t locsize; // Current size of the local stack

#include "node.c"
#include "expression.c"
#include "statement.c"

int16_t main( int16_t argc, int8_t ** argv )
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
    }
}
