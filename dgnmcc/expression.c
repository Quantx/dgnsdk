const unsigned int8_t prectbl[] = {
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
    Dot     // 13 - Highest
};

int8_t getPrec( unsigned int8_t op )
{
    int8_t i;
    for ( i = 0; i < sizeof(prectbl) && op >= prectbl[i]; i++ );
    return --i;
}

// Build and return an expression tree
struct mccnode * expr(struct mccnsp * curnsp)
{
    unsigned int8_t ostk[MAX_EXPR_OPER];
    unsigned int16_t otop = 0;

    struct mccnode * nstk[MAX_EXPR_NODE];
    unsigned int16_t ntop = 0;

    /*
      mode 0 = Not unary
      mode 1 = Unary
      mode 2 = Done, collapse stack
    */
    unsigned int16_t mode = 1;

    while ( mode < 2 )
    {
        struct mccnode * n;
        int8_t grp = 0;
        unsigned int8_t prec;

        // We're done, goto mode 2 for cleanup
        if ( tk < Number && tk != '(' && tk != ')' && tk != '[' && tk != ']' && tk != '"' ) mode = 2;
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
            nstk[ntop]->val = tkVal;

            if ( tk == Named )
            {
                nstk[ntop]->name = sbrk(tkVal);
                if ( nstk[ntop]->name == SBRKFAIL ) mccfail("unable to allocate space for node name");

                int16_t i;
                for ( i = 0; i < tkVal; i++ ) nstk[ntop]->name[i] = tkStr[i];

                ntok();
                if ( tk == '(' ) // Function call
                {
                    // TODO
                }
            }
            else
            {
                if ( tk == LongNumber ) nstk[ntop]->valLong = tkLong;
                ntok();
            }

            mode = 0;
            ntop++;
        }
        else if ( tk == ']' || tk == ')' || mode == 2 )
        {
            if ( tk == ')' ) grp = '(';
            else if ( tk == ']' ) grp = '[';// Brak;

            while ( otop && ostk[otop - 1] != grp )
            {
                otop--;

#ifdef DEBUG_EXPR
                write( 2, "Pop1:", 5 );
                octwrite( 2, ostk[otop] );
                write( 2, "\r\n", 2 );
#endif

                // This should never happen
                if ( ostk[otop] == '(' || ostk[otop] == '[' /*Brak*/ ) mccfail("wrong matched parenthasis or bracket in expression");

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

                nstk[ntop++] = n;
            }

            if ( mode != 2 )
            {
                if ( !otop ) mccfail("no matching open parenthasis or bracket in expression");

                otop--;

                if ( ostk[otop] == '[' )
                {
                    n = sbrk(sizeof(struct mccnode));
                    if ( n == SBRKFAIL ) mccfail("unable to allocate array node");
                    n->type = '[';

                    if ( ntop < 2 ) mccfail("not enough nodes for array operator");
                    n->right = nstk[--ntop];
                    n->left  = nstk[--ntop];

                    nstk[ntop++] = n;
                }

                mode = 0;

                ntok();
            }
        }
        else
        {
            // Remember: '(' has a prec of -1
            prec = getPrec(tk);
            while ( otop && getPrec(ostk[otop - 1]) >= prec )
            {
                otop--;

#ifdef DEBUG_EXPR
                write( 2, "Pop2:", 5 );
                octwrite( 2, ostk[otop] );
                write( 2, "\r\n", 2 );
#endif

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

                nstk[ntop++] = n;
            }

            // Convert to unary operators as needed
            if (mode)
            {
                switch (tk)
                {
                    case PostInc: tk = PreInc; break;
                    case PostDec: tk = PreDec; break;
                    case Add: tk = Plus; break;
                    case Sub: tk = Minus; break;
                    case Mul: tk = Deref; break;
                    case And: tk = Inder; break;
                }
            }

#ifdef DEBUG_EXPR
            write( 2, "Push:", 5 );
            octwrite( 2, tk );
            write( 2, "\r\n", 2 );
#endif

            ostk[otop++] = tk;

            if ( tk != PostInc && tk != PostDec ) mode = 1;
            ntok();
        }
    }

    if ( ntop != 1 ) mccfail("Unable to resolve expression");
    return *nstk;
}
