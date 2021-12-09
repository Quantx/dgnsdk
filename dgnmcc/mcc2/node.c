unsigned int32_t cni;

int16_t fd;

#ifdef DEBUG_NODE
int16_t fd_dbg;

unsigned int32_t cfp; // Current file position
unsigned int32_t lnp; // Last node position
#endif

void mccfail( int8_t * msg )
{
    octwrite( 2, cni );
    write( 2, ":", 1 );

    die(msg);
}

struct mccstmt * node()
{
    cni++;

    struct mccstmt * out = sbrk(sizeof(struct mccstmt CAST_NAME));
    if ( out == SBRKFAIL ) mccfail( "unable to allocate room for new statement node" );

    int16_t rv = read(0, out, sizeof(struct mccstmt CAST_NAME));
    if ( rv < sizeof(struct mccstmt CAST_NAME) )
    {
#ifdef DEBUG_NODE
        if ( rv < 0 ) write( 2, "ERROR", 5 );
        else write( 2, "DONE", 4 );
#endif
        return NULL;
    }

#ifdef DEBUG_NODE
    lnp = cfp;
    cfp += rv;

    write( 2, "Node: ", 6 );

    int8_t * nn = tokenNames[out->oper];
    int16_t i;
    for ( i = 0; nn[i]; i++ );
    write( 2, nn, i );
#endif

    // Injest name
    if ( out->oper == Variable
    ||   out->oper == Label
    ||   out->oper == LabelExtern )
    {
        out->name = sbrk(out->val);
        if ( out->name == SBRKFAIL ) mccfail( "unable to allocate room for new statement node's name" );
        read( 0, out->name, out->val );

#ifdef DEBUG_NODE
        cfp += out->val;
        
        write( 2, " '", 2 );
        write( 2, out->name, out->val );
        write( 2, "'", 1 );
#endif
    }
    
#ifdef DEBUG_NODE
    write( 2, "\n", 1 );
#endif

    return out;
}
