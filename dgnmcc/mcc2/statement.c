unsigned int16_t stmtid; // Current statement ID

int16_t stmttop;
struct mccstmt * stmtstk[MAX_STATEMENT_STK];

void statement( struct mccstmt * st )
{
    struct mccstmt * ce;
    struct mccstmt * stop = stmtstk[stmttop - 1];

    switch ( st->oper )
    {
        case Label:
        case LabelExtern: // Function declaration
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

            unsigned int16_t i;
            for ( i = 0; i < st->val; i++ ) curfunc->name[i] = st->name[i];
            
            curfunc->vac = 0;
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
        case Allocate:
            locsize += st->val;
            // Ensure at least MAX_EVAL_STK is available
//            if ( locsize < (0xFF - MAX_EVAL_STK) ) zerosize = locsize;
            brk(st);
            break;
        case Unallocate:
            locsize -= st->val;
//            if ( locsize < zerosize ) zerosize = locsize;
            brk(st);
            break;
        case If:
            st->val = stmtid++;
            stmtstk[stmttop++] = st;
            
            generate(expr(ce = node())); // Process if statement's expression
            brk(ce);
            
            optr->op = OpIf;
            optr->s.id = st->val;
            optr++;
            
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
        case For: // Basically just syntactic sugar for while loops

        case While:
            break;
        case Do:
            break;
        case Switch:
            break;
        case Case:
            break;
        case Default:
            break;
        case Break:
            break;
        case Continue:
            break;
        case Return:
            generate(expr(ce = node())); // Process return statement's expression
            
            optr->op = OpReturn;
            optr++;
            
            emitOpBuffer();
            
            brk(st);
            break;
        case End:
            stmttop--;
            
            if ( stmttop == -1 ) mccfail("statement stack underflow");
            
            if ( st->val == 1 ) // End of do-while loop
            {
                // TODO
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
