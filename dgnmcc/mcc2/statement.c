unsigned int16_t stmtid; // Current statement ID

// stmttop is defined in regalloc.c
struct mccfcst stmtstk[MAX_STATEMENT_STK];

void statement( struct mccstmt * st )
{
    struct mccstmt * ce;
    struct mccstmt * stop = stmttop ? stmtstk[stmttop - 1].st : NULL;
    struct mcceval * ev;

    switch ( st->oper )
    {
        case Label:
        case LabelExtern: // Function declaration
        {
            locsize = 0; // Reset local allocation size
            
            if ( curfunc ) brk2( curfunc ); // Effectively nuke break region 2

            curfunc = sbrk2(sizeof(struct mccfunc CAST_NAME));
            if ( curfunc == SBRKFAIL ) mccfail( "cannot allocate room for new function" );
            
//            (*functail)->next = curfunc;
//            functail = &curfunc;
            
            curfunc->vartbl = NULL;
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

            if ( fd_dbg > 0 )
            {
                write( fd_dbg, "*** EOF\n", 8 );
                close( fd_dbg );
            }
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
            if ( !st->val )
            {
                brk(st);
                break;
            }
            
            locsize -= st->val;
            
            // Can't actually unallocate anything as it might also unallocate a global variable
            struct mccvar ** dv;
            for ( dv = &curfunc->vartbl; *dv; dv = &(*dv)->next )
            {
                struct mccvar * cv = *dv;
                // Ignore global variables and local variables that are still in scope
                if ( cv->len || cv->addr < locsize ) continue;
                
                *dv = cv->next; // Drop from list
                
                // Update tail if it points to a node being dropped
                if ( curfunc->vartail == &cv->next ) curfunc->vartail = dv;
            }
            
            brk(st);
            break;
        }
        case If:
            st->val = stmtid++;
            stmtstk[stmttop  ].st = st;
            stmtstk[stmttop++].ev = NULL;
            
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
            if ( !stop || stop->oper != If ) mccfail("ELSE does not follow IF");
            
            // Spill at the end of a block which may never execute
            spillBlock( stmttop - 1, curfunc->z_size );
            
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
            
//            emitOpBuffer();
            
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
                // Increment stmttop before generating the condition to ensure it's on the same spill level
                stmtstk[stmttop++].st = st;
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
            if (!isDW) stmtstk[stmttop - 1].ev = st->oper == For ? expr(node()) : NULL;
            
            emitOpBuffer();
            break;
        }
        case Do:
            // Start of loop
            optr->op = OpStart;
            optr->s.id = stmtid;
            optr++;
            
            st->val = stmtid++;
            
            stmtstk[stmttop  ].st = st;
            stmtstk[stmttop++].ev = NULL;
            
            // The rest of this statement is handled by the following While statement
            emitOpBuffer();
            break;
        case Switch:
            st->val = stmtid++;
            stmtstk[stmttop  ].st = st;
            stmtstk[stmttop  ].val = 0;
            stmtstk[stmttop++].ev = NULL;
            
            generate(ev = expr(ce = node())); // Process switch statement's expression
            
            optr->op = OpSwitch;
            optr->s.id = st->val;
            
            // Location of the result to evaluate
            optr->reg  = ev->op->reg;
            optr->size = ev->op->size;
            
            optr++;
            
            brk(ce);
            
            emitOpBuffer();
            break;
        case Case:
        case Default:
        {
            int16_t i;
            struct mccfcst * cs;
        
            // Find most recent switch in stack
            for ( i = stmttop - 1, cs = NULL; i >= 0; i-- )
            {
                if ( stmtstk[i].st->oper == Switch )
                {
                    cs = stmtstk + i;
                    break;
                }
            }
            
            if ( !cs ) mccfail("Case or Default outside of switch");

            spillBlock( stmttop, curfunc->z_size );
        
            optr->op = st->oper == Case ? OpCase : OpDefault;
            optr->s.id = cs->st->val;
            optr->s.sid = cs->val++;
            
            optr->size = st->val;
            
            optr++;
            
            emitOpBuffer();
            brk(st);
            break;
        }
        case Break:
        case Continue:
        {
            int16_t i;
            struct mccfcst * cl;
        
            // Find most recent loop in stack
            for ( i = stmttop - 1, cl = NULL; i >= 0; i-- )
            {
                unsigned int8_t csop = stmtstk[i].st->oper;
                // Loop statements
                if ( (csop >= For && csop <= Do)
                // Switch statement
                || (st->oper == Break && csop == Switch) )
                {
                    cl = stmtstk + i;
                    break;
                }
            }
            
            if ( !cl ) mccfail("Break or Continue outside of loop");
            
            spillBlock( stmttop - 1, curfunc->z_size );
            
            optr->op = st->oper == Break ? OpBreak : OpContinue;
            optr->s.id = cl->st->val;
            optr++;
            
            emitOpBuffer();
            
            brk(st);
            break;
        }
        case Return:
            // If st->val == 1 then it's a "return;" statement
            // If st->val == 0 then it's a "return <expression>;" statement
            if ( !st->val )
            {
                generate(ev = expr(ce = node())); // Process return statement's expression
                
                // Location of the result to return
                optr->reg  = ev->op->reg;
                optr->size = ev->op->size;
            }
            
            optr->op = OpReturn;
            optr->s.id = 0;
            
            optr++;
            
            emitOpBuffer();
            
            brk(st);
            break;
        case End:
            if ( !stop ) mccfail("statement stack underflow");
            stmttop--;
            
            unsigned int16_t sop = stop->oper;

            // Spill upon exit of a block that may never execute
            if ( sop == If || sop == For || sop == While || sop == Do || sop == Switch )
            {
                if ( sop == For ) // Generate iterative portion of the for loop
                {
                    generate(stmtstk[stmttop].ev);
                }
            
                spillBlock( stmttop, curfunc->z_size );
            }
            
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
