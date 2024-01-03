#include "mcc2.h"

#include "../../lib/utils.c"

#include "node.c"
#include "function.c"
#include "expression.c"
#include "regalloc.c"
#include "generator.c"
#include "statement.c"

int16_t main( int16_t argc, int8_t ** argv )
{
#ifdef DEBUG
    if ( (sizeof(opNames) / sizeof(*opNames)) != _OpListCount ) mccfail( "OpNames length doesn't match OpList legnth" );
#endif

    br2 = sbrk(MAX_ANALYSIS_BRK);
    if ( br2 == SBRKFAIL ) mccfail( "unable to allocate room for analysis brk region" );

    cni--; // First node must be zero

    struct mccstmt * st;
    while ( st = node() ) // Analysis pass
    {
        statement(st);
    }

/*    
    brk(br2 + bp2); // Unallocate unused space from break region 2

    while ( st = node() ) // Code Generation Pass
    {
        statement(st);
    }
*/
}
