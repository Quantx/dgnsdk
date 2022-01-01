// Break region 2
void * br2;
unsigned int16_t bp2;

void * sbrk2( unsigned int16_t size )
{
    void * out = br2 + bp2;

    bp2 += size;
    
    if ( bp2 > MAX_ANALYSIS_BRK ) return SBRKFAIL;
    
    return out;
}

void brk2( void * addr )
{
    if ( addr < br2 ) mccfail( "pointer lies below analysis brk region" );
    if ( addr - br2 > MAX_ANALYSIS_BRK ) mccfail( "pointer lies above analysis brk region" );
    
    bp2 = addr - br2;
}

struct mccfunc * curfunc, * functbl, ** functail = &functbl;

struct mccfunc * getFunc( struct mccstmt * st )
{
    struct mccfunc * cf;
    
    for ( cf = functbl; cf; cf = cf->next )
    {
        if ( st->val == cf->len )
        {
            unsigned int16_t i;
            for ( i = 0; i < st->val; i++ )
            {
                if ( st->name[i] != cf->name[i] ) break;
            }
            
            if ( i == st->val ) return cf;
        }
    }
    
    return NULL;
}

struct mccvar * getVarFunc( struct mccstmt * st, struct mccfunc * fn )
{
    struct mccvar * cv;
    
    if ( st->oper == VariableLocal ) // Local
    {
        for ( cv = fn->vartbl; cv; cv = cv->next )
        {
            if ( !cv->len && st->val == cv->addr ) return cv;
        }
    }
    else if ( st->oper == Variable ) // Global
    {
        for ( cv = fn->vartbl; cv; cv = cv->next )
        {
            if ( cv->len == st->val )
            {
                unsigned int16_t i;
                for ( i = 0; i < st->val; i++ )
                {
                    if ( st->name[i] != cv->name[i] ) break;
                }
                
                if ( i == st->val ) return cv;
            }
        }
    }
#ifdef DEBUG
    else
    {
        write( 2, "Non-variable type in getVar: ", 29 );

        int8_t * nn = tokenNames[st->oper];
        int16_t i;
        for ( i = 0; nn[i]; i++ );
        write( 2, nn, i );
        write( 2, "\n", 1 );
    }
#endif
    
    return NULL;
}

struct mccvar * getVar( struct mccstmt * st ) { return getVarFunc( st, curfunc ); }

int16_t regAlloc( struct mccvar * v )
{

}
