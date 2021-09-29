void statement( struct mccstmt * st )
{
    struct mccstmt * ce;

    switch ( st->oper )
    {
        case LabelExtern: // Function declaration
        case Label:
            locsize = 0; // Reset local allocation size
            break;
        case Allocate:
            locsize += st->val;
            // Ensure at least MAX_EVAL_STK is available
            if ( locsize < (0xFF - MAX_EVAL_STK) ) zerosize = locsize;
            break;
        case Unallocate:
            locsize -= st->val;
            if ( locsize < zerosize ) zerosize = locsize;
            break;
        case If:
            st->val = stmtid++;
            expr(ce = node()); // Get If statement's expression
            break;
        case Else:
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
            break;
        case End:
            break;
        default: // Expression (NOTE: might be a "Void" expression)
            expr(st);
            break;
    }
}
