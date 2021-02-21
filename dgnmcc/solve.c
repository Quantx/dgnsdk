int16_t solve( struct mccnode * t )
{
    int16_t solved = 0, last_solved = -1;

    while ( last_solved != solved )
    {
        struct mccnode * n = t, * ns[MAX_EXPR_NODE];
        int16_t nspos = 0;

        last_solved = solved;
        solved = 0;

        for ( solved = 0; n || nspos; ns[nspos++] = n, n = n->left )
        {
            if ( !n ) { n = ns[--nspos]->right; continue; }

            switch ( n->type )
            {
                case Tern:
                case LogOr:
                case LogAnd:
                case Or:
                case Xor:
                case And:
                case Eq:
                case Neq:
                case Less:
                case Great:
                case LessEq:
                case GreatEq:
                case Shl:
                case Shr:
                case Add:
                case Sub:
                case Mul:
                case Div:
                case Mod:
                case Sizeof:
                case LogNot:
                case Not:
                case Plus:
                case Minus:
                case Inder:
                case Deref:
                case PreInc:
                case PreDec:
                case Dot:
                case Arrow:
                case PostInc:
                case PreInc:
            }

            ns[nspos++] = n;
            n = n->left;
        }
    }
}
