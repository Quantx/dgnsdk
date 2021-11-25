unsigned int32_t cni;

struct funcstat
{
    int8_t * name;
    unsigned int16_t len;

    int16_t fd;
#ifdef DEBUG
    int16_t fd_dbg;
#endif
} curfunc;

void mccfail( int8_t * msg )
{
    octwrite( 2, cni );
    write( 2, ":", 1 );

    die(msg);
}

struct mccstmt * node()
{
    struct mccstmt * out = sbrk(sizeof(struct mccstmt CAST_NAME));
    if ( out == SBRKFAIL ) mccfail( "unable to allocate room for new statement node" );

    if ( read( 0, out, sizeof(struct mccstmt CAST_NAME) ) <= 0 ) return NULL;

    // Injest name
    if ( out->oper == Variable
    ||   out->oper == Label
    ||   out->oper == LabelExtern )
    {
        out->name = sbrk(out->val);
        if ( out->name == SBRKFAIL ) mccfail( "unable to allocate room for new statement node's name" );
        read( 0, out->name, out->val );
    }

    cni++;

    return out;
}
