unsigned int16_t stmtid; // Current statement ID

unsigned int16_t stmttop;
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
            if ( curfunc.fd > 0 ) close( curfunc.fd ); // Close existing function

#ifdef DEBUG
            if ( curfunc.fd_dbg > 0 ) close( curfunc.fd_dbg );
#endif

            curfunc.name = st->name;
            curfunc.len = st->val;
            
            int8_t * fpath = sbrk(st->val + 6);
            if (fpath == SBRKFAIL) mccfail("unable to allocate room for function file name");
            
//            write( 2, curfunc.name, curfunc.len );
//            write( 2, "\n", 1 );
            
            unsigned int16_t i;
            for ( i = 0; i < st->val; i++ ) fpath[i] = curfunc.name[i];
            fpath[i] = '.';
            fpath[i+1] = 'f';
            fpath[i+2] = 'u';
            fpath[i+3] = 'n';
            fpath[i+4] = 'c';
            fpath[i+5] = '\0';
            
//            write( 2, fpath, st->val + 6 );
//            write( 2, "\n", 1 );
            
            if ( (curfunc.fd = creat( fpath, 0666 )) < 0 ) mccfail("unable to creat file for new function");
            
#ifdef DEBUG
            fpath[i] = '.';
            fpath[i+1] = 'd';
            fpath[i+2] = 'e';
            fpath[i+3] = 'b';
            fpath[i+4] = 'g';
            fpath[i+5] = '\0';
            
//            write( 2, fpath, st->val + 6 );
//            write( 2, "\n", 1 );
            
            if ( (curfunc.fd_dbg = creat( fpath, 0666 )) < 0 ) mccfail("unable to creat debug file for new function");
#endif
            //brk(fpath); // handled by the next brk() statement

            brk(st);
            break;
        case Allocate:
            locsize += st->val;
            // Ensure at least MAX_EVAL_STK is available
            if ( locsize < (0xFF - MAX_EVAL_STK) ) zerosize = locsize;
            brk(st);
            break;
        case Unallocate:
            locsize -= st->val;
            if ( locsize < zerosize ) zerosize = locsize;
            brk(st);
            break;
        case If:
            st->val = stmtid++;
            stmtstk[stmttop++] = st;
            
            expr(ce = node()); // Process if statement's expression
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
            expr(ce = node()); // Process return statement's expression
            
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
            expr(st);
            emitOpBuffer();
            
            brk(st);
            break;
    }
}
