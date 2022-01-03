#ifdef DEBUG_ALLOC
void writeVar(int16_t fd, struct mccvar * wv)
{
    if ( wv->len ) write( fd, wv->name, wv->len );
    else decwrite( fd, wv->addr );
    
    write( fd, ":", 1 );
    
    switch ( wv->flags & VAR_ALC_MASK )
    {
        case VAR_ALC_ZP: write( fd, "zeropage", 9 ); break;
        case VAR_ALC_MM: write( fd, "main mem", 9 ); break;
        case VAR_ALC_DA: write( fd, "dontaloc", 9 ); break;
        case VAR_ALC_NA: write( fd, "not aloc", 9 ); break;
    }
    
    write( fd, ":z_addr:", 8 );
    decwrite( fd, wv->z_addr );
    write( fd, ":s_addr:", 8 );
    decwrite( fd, wv->s_addr );
    write( fd, ":size:", 6 );
    decwrite( fd, wv->size );
}
#endif

void spill(struct mccvar * sv, unsigned int8_t spr)
{
    if ( (sv->flags & VAR_ALC_MASK) != VAR_ALC_ZP ) return;

    if ( sv->len ) // Global variable
    {
        // Generate spill instruction
        optr->op = OpAddrGlb;
        optr->a.name = sv->name;
        optr->a.val = sv->len;
    }
    else // Local variable
    {
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
        unsigned int8_t opc = opClass(ev->st);
        
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

                // Don't allocate constant, or large types
                unsigned int8_t tf = ev->st->type & IR_TYPE_MASK;
                if ( tf >= IR_STRUC && tf <= IR_FUNC ) av->flags = VAR_ALC_DA;
                else av->flags = VAR_ALC_NA;
            }
            
            ev->var = av;
            
            av->lac = curfunc->vac++;
            
            unsigned int8_t af = av->flags & VAR_ALC_MASK;
            // Number of registers occupied by this variable
            unsigned int8_t z_size = av->size >> 1;
            if ( !z_size ) z_size++;

            
            if ( pv->st->oper == Inder ) // Address-of, spill this variable
            {
                #ifdef DEBUG_ALLOC
                write( 2, "DA Var: ", 8 );
                writeVar( 2, av );
                write( 2, "\n", 1 );
                #endif
            
                spill(av, curfunc->z_size);
                av->flags = VAR_ALC_DA; // Mark as "do not allocate"
            }
            // Variable is in need of allocation
            else if ( af == VAR_ALC_MM || af == VAR_ALC_NA )
            {
                // There is room left so allocate variable to end of current zero page region
                if ( curfunc->z_size + z_size <= MAX_REG )
                {
                    av->z_addr = curfunc->z_size;
                    curfunc->z_size += z_size;
                    if ( curfunc->z_size > curfunc->z_max ) curfunc->z_max = curfunc->z_size;
                }
                // We're full, make some room
                else
                {
                    unsigned int16_t aac = -1; // Average access count
                    unsigned int16_t aap = 0; // Avearage access position

                    for ( av->z_addr = 0; av->z_addr <= MAX_REG - z_size; av->z_addr++ )
                    {
                        unsigned int16_t cac = 0; // Current access count
                        unsigned int16_t ncv = 0; // Number of conflicting variables
                    
                        struct mccvar * cv;
                        for ( cv = curfunc->vartbl; cv; cv = cv->next )
                        {
                            // Check if this variable is allocated to zero page and that the current av address lies inside this variable's allocation range
                            unsigned int8_t cvzs = cv->size >> 1;
                            if (!cvzs) cvzs++;
                            
                            if ( (cv->flags & VAR_ALC_MASK) == VAR_ALC_ZP // Zero page resident variables only
                            &&   av->z_addr <= cv->z_addr + cvzs && cv->z_addr <= av->z_addr + z_size ) // Conflicting address range
                            {
                                cac += cv->lac;
                                ncv++;
                            }
                        }
                        
                        cac /= ncv;
                        if ( cac < aac ) // Update aac & aap
                        {
                            aac = cac;
                            aap = av->z_addr;
                        }
                    }
                    
                    av->z_addr = aap;
                    
                    // Update zero page size if needed
                    if ( av->z_addr + z_size > curfunc->z_size ) curfunc->z_size = av->z_addr + z_size;
                    
                    // Spill conflicting variables
                    struct mccvar * sv;
                    for ( sv = curfunc->vartbl; sv; sv = sv->next )
                    {
                        unsigned int8_t svzs = sv->size >> 1;
                        if (!svzs) svzs++;
                    
                        if ( (sv->flags & VAR_ALC_MASK) == VAR_ALC_ZP // Zero page resident variables only
                        &&   av->z_addr <= sv->z_addr + svzs && sv->z_addr <= av->z_addr + z_size ) // Conflicting address range
                        {
                            spill(sv, curfunc->z_size);
                        }
                    }
                }

                av->flags = VAR_ALC_ZP; // Make sure to spill all conflicting variables, before marking this as no longer in the stack
                
                if ( af == VAR_ALC_MM )
                {
                    // Generate allocation instruction
                    if ( av->len ) // Global variable
                    {
                        optr->op = OpAddrGlb;
                        optr->a.name = av->name;
                        optr->a.val = av->len;
                    }
                    else // Local variable
                    {
                        optr->op = OpAddrLoc;
                        optr->a.val = av->s_addr;
                    }

                    optr->reg = curfunc->z_size;
                    optr->size = IR_PTR_SIZE;

                    optr++;

                    optr->op = OpLoad;
                    optr->reg = av->z_addr;
                    optr->size = av->size;
                    
                    optr->c.r_arg = curfunc->z_size;
                    optr->c.r_size = av->size;
                    
                    optr++;
                }
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
