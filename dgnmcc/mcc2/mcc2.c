#include "mcc2.h"

unsigned int16_t curfno;

#include "../../lib/utils.c"

unsigned int8_t zerosize; // Current size of the zero page resident portion of the local stack
unsigned int16_t locsize; // Current size of the local stack

unsigned int32_t stmtid;


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
