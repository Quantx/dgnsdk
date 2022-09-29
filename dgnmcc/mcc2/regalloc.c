int16_t stmttop;

#ifdef DEBUG_ALLOC
void writeVar(int16_t fd, struct mccvar * wv)
{
    if ( wv->len ) write( fd, wv->name, wv->len );
    else decwrite( fd, wv->addr );
    
    write( fd, ":", 1 );
    
    switch ( wv->flags & VAR_ALC_MASK )
    {
        case VAR_ALC_ZP: write( fd, "zeropage", 9 ); break;
        case VAR_ALC_MM: write( fd, "main_mem", 9 ); break;
        case VAR_ALC_DA: write( fd, "dontaloc", 9 ); break;
        case VAR_ALC_NA: write( fd, "not_aloc", 9 ); break;
    }
    
    write( fd, ":reg:", 5 );
    decwrite( fd, wv->reg );
//    write( fd, ":addr:", 6 );
//    decwrite( fd, wv->addr );
    write( fd, ":size:", 6 );
    decwrite( fd, wv->size );
}
#endif

/*
// sv = variable to spill
// spr = temp register used to store spill address
void spill(struct mccvar * sv, unsigned int8_t spr)
{
    // Can only spill a variable that's allocated to zero page
    if ( (sv->flags & VAR_ALC_MASK) != VAR_ALC_ZP ) return;

    if ( sv->len ) // Global variable
    {
        write( 2, "Spilled global: ", 16 );
        write( 2, sv->name, sv->len );
    
        // Generate spill instruction
        optr->op = OpAddrGlb;
        optr->a.name = sv->name;
        optr->a.val = sv->len;
    }
    else // Local variable
    {
        write( 2, "Spilled local: ", 15 );
        decwrite( 2, sv->addr );
    
        // Find room on stack for this variable
        for ( sv->s_addr = 0; 1; sv->s_addr++ )
        {
            struct mccvar * cv;
            for ( cv = curfunc->vartbl; cv; cv = cv->next )
            {
                unsigned int8_t cf = cv->flags & VAR_ALC_MASK;
                if ( !cv->len && (cf == VAR_ALC_MM || cf == VAR_ALC_DA) // Stack resident variables
                &&   sv->s_addr <= cv->s_addr + cv->size && cv->s_addr <= sv->s_addr + sv->size ) // Conflicting address
                {
                   break; // This address won't work, try another one
                }
            }
            
            if (!cv) break; // No conflicting variables, this address works
        }
        
        // Generate spill instruction
        optr->op = OpAddrLoc;
        optr->a.val = sv->s_addr;
    }
    
    write( 2, "\n", 1 );

    // Generate rest of spill instruction
    optr->reg = spr;
    optr->size = IR_PTR_SIZE;

    optr++; 

    optr->op = OpStore;
    optr->reg = sv->z_addr;
    optr->size = sv->size;
    
    optr->c.r_arg = spr;
    optr->c.r_size = sv->size;
    
    optr++;
    
    sv->flags = VAR_ALC_MM;
}
*/

// Spill all variables allocated within the last block
void spillBlock(int16_t level, unsigned int8_t spr)
{
    struct mccvar * cv;
    for ( cv = curfunc->vartbl; cv; cv = cv->next )
    {
        // Check if the variable is above the current level and allocated in zp
        if ( cv->stmt >= level && (cv->flags & VAR_ALC_MASK) == VAR_ALC_ZP ) cv->flags = VAR_ALC_MM;
    }
}

/* No need to spill globals, since the we're using indirection for everything now which solves the aliasing problem

// Spill all allocated global variables
void spillGlobals(unsigned int8_t spr)
{
    struct mccvar * cv;
    for ( cv = curfunc->vartbl; cv; cv = cv->next )
    {
        // Check if the variable is global and allocated in zp
        if ( cv->len && (cv->flags & VAR_ALC_MASK) == VAR_ALC_ZP ) cv->flags = VAR_ALC_MM;
    }
}
*/

void regalloc(struct mcceval * cn)
{
    struct mcceval * ev, * ln = NULL; // Current and last nodes

    while ( cn || etop ) // Post order traversal
    {
        if ( cn )
        {
            estk[etop++] = cn;
            cn = cn->left;
            continue;
        }
        
        ev = estk[etop - 1];
        
        if ( ev->right && ln != ev->right )
        {
            cn = ev->right;
            continue;
        }
        
        ln = ev; etop--;

        struct mcceval * pv = ev->parent;

        unsigned int8_t op = ev->st->oper;
        //unsigned int8_t opc = opClass(ev->st);
        
        if ( op == Variable || op == VariableLocal )
        {
            struct mccvar * av = getVar(ev->st);
            
            if ( !av ) // Create new variable
            {
                av = sbrk2(sizeof(struct mccvar CAST_NAME));
                if (av == SBRKFAIL) mccfail( "unable to allocate room for new variable" );
                *(curfunc->vartail) = av;
                curfunc->vartail = &av->next;
                av->next = NULL;
                
                if ( op == Variable )
                {
                    av->len = ev->st->val;
                    av->name = sbrk2(av->len);
                    
                    if (av->name == SBRKFAIL) mccfail( "unable to allocate room for new variable name" );
                    
                    unsigned int16_t i;
                    for ( i = 0; i < av->len; i++ ) av->name[i] = ev->st->name[i];
                }
                else
                {
                    av->len = 0;
                    av->addr = ev->st->val;
                }
                
                av->size = ev->st->size;
                av->flags = VAR_ALC_NA;
/*
                // Don't allocate constant, or large types
                unsigned int8_t tf = ev->st->type & IR_TYPE_MASK;
                if ( tf >= IR_STRUC && tf <= IR_FUNC ) av->flags = VAR_ALC_DA;
                else av->flags = VAR_ALC_NA;
*/
            }
            
            ev->var = av;
            
            av->lac = curfunc->vac++;
            
            unsigned int8_t af = av->flags & VAR_ALC_MASK;
            if ( af == VAR_ALC_MM || af == VAR_ALC_NA )
            {
                // There is room left so allocate variable to end of current zero page region
                if ( curfunc->z_size + 1 <= MAX_REG )
                {
                    av->reg = curfunc->z_size;
                    curfunc->z_size++;// += z_size;
//                    if ( curfunc->z_size > curfunc->z_max ) curfunc->z_max = curfunc->z_size;
                }
                // We're full, find the register which was accessed the longest ago
                else
                {
                    struct mccvar * cv, * oav;
                
                    for ( cv = curfunc->vartbl; cv; cv = cv->next )
                    {
                        // Only look at variables which are currently held in zero page
                        if ( (cv->flags & VAR_ALC_MASK) == VAR_ALC_ZP
                        // Check if this variable was last accessed longer ago
                        && (cv == curfunc->vartbl || cv->lac < oav->lac) ) oav = cv;
                    }
                    
                    if (!oav) mccfail("Failed make room for new variable in zero page, vartbl was empty");
                    
                    // Spill oldest variable
                    oav->flags = VAR_ALC_MM;
                    
                    av->reg = oav->reg;
                }

                av->stmt = stmttop - 1;
                av->flags = VAR_ALC_ZP; // Make sure to spill all conflicting variables, before marking this as no longer in the stack
                
                // Load the address of this new variable into the allocated register
                if ( av->len ) // Global variable
                {
                    optr->op = OpAddrGlb;
                    optr->a.name = av->name;
                    optr->a.val = av->len;
                }
                else // Local variable
                {
                    optr->op = OpAddrLoc;
                    optr->a.val = av->addr;
                }

                optr->reg = av->reg;
                optr->size = IR_PTR_SIZE;
                optr->flags = 0;

                optr++;
            }
        }
    }
#ifdef DEBUG_ALLOC    
    struct mccvar * cv;
    for ( cv = curfunc->vartbl; cv; cv = cv->next )
    {
        writeVar(2, cv);
        write(2, "\n", 1);
    }
#endif
}
