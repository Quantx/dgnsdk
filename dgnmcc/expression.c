const unsigned char prectbl[] = {
    Ass,    // 0 - Lowest
    Tern,   // 1
    LogOr,  // 2
    LogAnd, // 3
    Or,     // 4
    Xor,    // 5
    And,    // 6
    Eq,     // 7
    Less,   // 8
    Shl,    // 9
    Add,    // 10
    Mul,    // 11
    LogNot, // 12
    Brak    // 13 - Highest
};

unsigned char getPrec( unsigned char op )
{
    unsigned char i;
    for ( i = 0; op >= prectbl[i]; i++ );
    return --i;
}

// Build and return an expression tree
struct mccnode * expr(struct mccnsp * curnsp)
{
    unsigned char ostk[MAX_EXPR_OPER];
    unsigned int otop = 0;

    struct mccnode * nstk[MAX_EXPR_NODE];
    unsigned int ntop = 0;

    // Is this a unary operator?
    unsigned int unary = 1;

    ntok();
    while ( tk >= Number || tk == '(' || tk == ')' || tk == ']' || tk == '"' )
    {
/*
	(
               !stC.empty()
            && stC.top() != '('
            && (
                   (s[i] != '^' && p[stC.top()] >= p[s[i]])
                || (s[i] == '^' && p[stC.top()] >  p[s[i]])
            )
        )
*/
        if ( tk == Number || tk == LongNumber || tk == '"' || tk == Named )
        {
            nstk[ntop] = sbrk(sizeof(struct mccnode));
            if ( nstk[ntop] == SBRKFAIL ) mccfail("unable to allocate space for new node");
            nstk[ntop]->left = nstk[ntop]->right = NULL;
            nstk[ntop]->type = tk;

            if ( tk == Named )
            {
                // TODO
                // Handle function calls here as well
            }
            else if ( tk == LongNumber ) nstk[ntop]->valLong = tkLong;

            unary = 0;
        }
        else
        {
            struct mccnode * n;
            char grp = ')';
            unsigned char prec;

            switch (tk)
            {
                case ']':
                    grp = Brak;
                case ')':
                    while ( otop && ostk[--otop] != grp )
                    {
                        // This should never happen
                        if ( ostk[otop] == '(' || ostk[otop] == Brak ) mccfail("wrong matched parenthasis or bracket in expression");

                        n = sbrk(sizeof(struct mccnode));
                        if ( n == SBRKFAIL ) mccfail("unable to allocate expression node");

                        n->type = ostk[otop];

                        // Prefix unary operators
                        if ( n->type == PreInc || n->type == PreDec
                        ||   n->type == Plus   || n->type == Minus
                        ||   n->type == Inder  || n->type == Deref
                        ||   n->type == LogNot || n->type == Not )
                        {
                            if ( ntop < 1 ) mccfail("not enough nodes for pre-operator");
                            n->right = nstk[--ntop];
                            n->left  = NULL;
                        }
                        // Postfix unary operators
                        else if ( n->type == PostInc || n->type == PostDec )
                        {
                            if ( ntop < 1 ) mccfail("not enough nodes for post-operator");
                            n->right = NULL;
                            n->left  = nstk[--ntop];
                        }
                        else
                        {
                            if ( ntop < 2 ) mccfail("not enough nodes for operator");
                            n->right = nstk[--ntop];
                            n->left  = nstk[--ntop];
                        }
                    }

                    if ( !otop ) mccfail("no matching open parenthasis or bracket in expression");

                    unary = 0;
                    break;
                default:
                    // Remember: '(' has a prec of -1
                    prec = getPrec(tk);
                    while ( otop && getPrec(ostk[--otop]) >= prec )
                    {
                        n = sbrk(sizeof(struct mccnode));
                        if ( n == SBRKFAIL ) mccfail("unable to allocate expression node");

                        n->type = ostk[otop];

                        // Prefix unary operators
                        if ( n->type == PreInc || n->type == PreDec
                        ||   n->type == Plus   || n->type == Minus
                        ||   n->type == Inder  || n->type == Deref
                        ||   n->type == LogNot || n->type == Not )
                        {
                            if ( ntop < 1 ) mccfail("not enough nodes for pre-operator");
                            n->right = nstk[--ntop];
                            n->left  = NULL;
                        }
                        // Postfix unary operators
                        else if ( n->type == PostInc || n->type == PostDec )
                        {
                            if ( ntop < 1 ) mccfail("not enough nodes for post-operator");
                            n->right = NULL;
                            n->left  = nstk[--ntop];
                        }
                        else
                        {
                            if ( ntop < 2 ) mccfail("not enough nodes for operator");
                            n->right = nstk[--ntop];
                            n->left  = nstk[--ntop];
                        }
                    }
                case '(':
                    // Convert to unary operators as needed
                    if (unary) switch (tk)
                    {
                        case PostInc: tk = PreInc; break;
                        case PostDec: tk = PreDec; break;
                        case Add: tk = Plus; break;
                        case Sub: tk = Minus; break;
                        case Mul: tk = Deref; break;
                        case And: tk = Inder; break;
                    }

                    ostk[otop++] = tk;

                    unary = 1;
                    break;
            }
        }
    }

    return *nstk;
}
