unsigned int16_t stmtid; // Current statement ID

int16_t stmttop;
struct mccfcst * stmtstk[MAX_STATEMENT_STK];

void statement( struct mccstmt * st )
{
    struct mccstmt * ce;
    // Technically an underflow if the stack is empty, but who gives a shit
    struct mccstmt * stop = stmtstk[stmttop - 1]->st;
    struct mcceval * ev;

    switch ( st->oper )
    {
        case Label:
        case LabelExtern: // Function declaration
        {
            locsize = 0; // Reset local allocation size
            
            if ( curfunc ) brk2( curfunc );

            curfunc = sbrk2(sizeof(struct mccfunc CAST_NAME));
            if ( curfunc == SBRKFAIL ) mccfail( "cannot allocate room for new function" );
            
//            (*functail)->next = curfunc;
//            functail = &curfunc;
            
            curfunc->vartail = &curfunc->vartbl;
            
            curfunc->name = sbrk2(st->val);
            if ( curfunc->name == SBRKFAIL ) mccfail( "cannot allocate room for new function name" );
            curfunc->len = st->val;

            int16_t i;
            for ( i = 0; i < st->val; i++ ) curfunc->name[i] = st->name[i];
            
            curfunc->vac = 0;
            curfunc->z_max = 0;
            curfunc->z_size = 0;
            curfunc->s_size = 0;
            
            // Last thing allocated should be the statement's name
            if (sbrk(6) == SBRKFAIL) mccfail("unable to allocate room for function file name");
            
            st->name[st->val  ] = '.';
            st->name[st->val+1] = 'f';
            st->name[st->val+2] = 'u';
            st->name[st->val+3] = 'n';
            st->name[st->val+4] = 'c';
            st->name[st->val+5] = '\0';
            
            if ( fd > 0 ) close( fd ); // Close existing function
            if ( (fd = creat( st->name, 0666 )) < 0 ) mccfail("unable to creat file for new function");
            
#ifdef DEBUG
            st->name[st->val  ] = '.';
            st->name[st->val+1] = 'd';
            st->name[st->val+2] = 'e';
            st->name[st->val+3] = 'b';
            st->name[st->val+4] = 'g';
            st->name[st->val+5] = '\0';

            if ( fd_dbg > 0 ) close( fd_dbg );
            if ( (fd_dbg = creat( st->name, 0666 )) < 0 ) mccfail("unable to creat debug file for new function");
#endif

            brk(st);
            break;
        }
        case Allocate:
            locsize += st->val;
            brk(st);
            break;
        case Unallocate:
        {
            locsize -= st->val;
            
            // Can't actually unallocate anything as it might also unallocate a global variable
            struct mccvar ** dv;
            for ( dv = &curfunc->vartbl; *dv; dv = &(*dv)->next )
            {
                struct mccvar * cv = *dv;
                // Ignore global variables and local variables that are still in scope
                if ( !cv->len || cv->addr < locsize ) continue;
                
                *dv = cv->next; // Drop from list
                
                // Update tail if it points to a node being dropped
                if ( curfunc->vartail == &cv->next ) curfunc->vartail = dv;
            }
            
            brk(st);
            break;
        }
        case If:
            st->val = stmtid++;
            stmtstk[stmttop  ]->st = st;
            stmtstk[stmttop++]->ev = NULL;
            
            generate(ev = expr(ce = node())); // Process if statement's expression
            
            optr->op = OpIf;
            optr->s.id = st->val;
            
            // Location of the result to evaluate
            optr->reg  = ev->op->reg;
            optr->size = ev->op->size;
            
            optr++;
            
            brk(ce);
            
            emitOpBuffer();
            break;
        case Else:
            if ( stop->oper != If ) mccfail("ELSE does not follow IF");
            
            optr->op = OpElse;
            optr->s.id = stop->val;
            optr++;
            
            emitOpBuffer();
            brk(st);
            break;
        case For:
            generate(expr(ce = node())); // Process initial statement
            brk(ce);
            // break; This intentionally feeds into the While block
        case While:
        {   
            unsigned int16_t csi;
            unsigned int16_t isDW = st->oper == While && st->val;
            
            // This is a do-while loop
            if (isDW) csi = stop->val;
            // Normal while (or for) loop
            else
            {
                // Start of loop
                optr->op = OpStart;
                optr->s.id = csi = stmtid++;
                optr++;
                
                st->val = csi;
                stmtstk[stmttop]->st = st;
            }
            
            // Process conditional statement
            generate(ev = expr(ce = node()));
            
            // Evaluate result of loop
            optr->op = OpWhile;
            optr->s.id = csi;
            
            // Location of the result to evaluate
            optr->reg  = ev->op->reg;
            optr->size = ev->op->size;
            
            optr++;
            
            brk(ce);
            
            // Store incremental statement for later (will be executed at the corresponding End statement)
            // Since this node is allocated after the current statement, unallocating the current statement handles cleanup of this too
            if (!isDW) stmtstk[stmttop++]->ev = st->oper == For ? expr(node()) : NULL;
            
            emitOpBuffer();
            break;
        }
        case Do:
            // Start of loop
            optr->op = OpStart;
            optr->s.id = stmtid;
            optr++;
            
            st->val = stmtid++;
            
            stmtstk[stmttop  ]->st = st;
            stmtstk[stmttop++]->ev = NULL;
            
            // The rest of this statement is handled by the following While statement
            emitOpBuffer();
            break;
        case Switch:
            // TODO switch statements
        
            break;
        case Case:
        
            break;
        case Default:
        
            break;
        case Break:
        case Continue:
        {
            int16_t i;
            struct mccfcst * cl;
        
            // Find most recent loop in stack
            for ( i = stmttop - 1, cl = NULL; i >= 0; i-- )
            {
                unsigned int8_t csop = stmtstk[i]->st->oper;
                // Loop statements
                if ( (csop >= For && csop <= Do)
                // Switch statement
                || (st->oper == Break && csop == Switch) )
                {
                    cl = stmtstk[i];
                    break;
                }
            }
            
            if ( !cl ) mccfail("Break or Continue outside of loop");
        
            optr->op = st->oper == Break ? OpBreak : OpContinue;
            optr->s.id = cl->st->val;
            optr++;
            
            emitOpBuffer();
            
            brk(st);
            break;
        }
        case Return:
            generate(ev = expr(ce = node())); // Process return statement's expression
            
            optr->op = OpReturn;
            optr->s.id = 0;
            
            // Location of the result to evaluate
            optr->reg  = ev->op->reg;
            optr->size = ev->op->size;
            
            optr++;
            
            emitOpBuffer();
            
            brk(st);
            break;
        case End:
            if ( !stmttop ) mccfail("statement stack underflow");
            stmttop--;
            
            optr->op = OpEnd;
            optr->s.id = stop->val;
            optr++;
            
            emitOpBuffer();
            brk(stop);
            break;
        default: // Expression (NOTE: might be a "Void" expression)
            generate(expr(st));
            emitOpBuffer();
            brk(st);
            break;
    }
}
