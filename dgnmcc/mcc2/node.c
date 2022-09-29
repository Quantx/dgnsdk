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

#ifdef DEBUG
    int16_t i = 0;
    while(msg[i])i++;
    write( 2, msg, i );
    int * a = 0, b = *a;
#endif    

    die(msg);
}

int16_t opClass( struct mccstmt * st )
{
    switch (st->type & IR_TYPE_MASK)
    {
        case IR_VOID:
            return OP_CLASS_VOID;
        case IR_CHR:
        case IR_INT:
        case IR_LNG:
            return OP_CLASS_SIGNED;
        case IR_UCHR:
        case IR_UINT:
        case IR_ULNG:
            return OP_CLASS_UNSIGNED;
        case IR_FPV:
        case IR_DBL:
            return OP_CLASS_FLOAT;
        case IR_PTR:
        case IR_ARRAY: // Remember, arrays aren't always pointers, try reconsidering this later
            return OP_CLASS_POINTER;
    }
    
    return -1;
}

struct mccstmt * node()
{
    cni++;

    struct mccstmt * out = sbrk(sizeof(struct mccstmt CAST_NAME));
    if ( out == SBRKFAIL ) mccfail( "unable to allocate room for new statement node" );

    int16_t rv = read(0, out, sizeof(struct mccstmt CAST_NAME));
    if ( rv < sizeof(struct mccstmt CAST_NAME) || out->oper == EndOfFile )
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

    int8_t * nn = mccTokenNames[out->oper];
    int16_t i;
    for ( i = 0; nn[i]; i++ );
    write( 2, nn, i );
#endif

    // Ingest name
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
