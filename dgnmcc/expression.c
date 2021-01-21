const unsigned int8_t prectbl[] = {
    Ass,    // 0 - Lowest
    Tern,   // 1 (Right associativity)
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
    Sizeof, // 12 (Right associativity)
    Dot     // 13 - Highest
};

int8_t getPrec( unsigned int8_t op )
{
    int8_t i;
    for ( i = 0; i < sizeof(prectbl) && op >= prectbl[i]; i++ );
    return --i;
}

unsigned int8_t ostk[MAX_EXPR_OPER];
unsigned int16_t otop;

struct mccnode * nstk[MAX_EXPR_NODE];
unsigned int16_t ntop;

void expr_reduce()
{
    otop--;

#ifdef DEBUG_EXPR
    write( 2, "Pop:", 4 );
    writeToken( 2, ostk[otop] );
    write( 2, "\r\n", 2 );
#endif

    // This should never happen
    if ( ostk[otop] == '(' || ostk[otop] == '[' || ostk[otop] == '?' )
    mccfail("wrong matched parenthasis or bracket in expression");

    struct mccnode * n = sbrk(sizeof(struct mccnode));
    if ( n == SBRKFAIL ) mccfail("unable to allocate expression node");

    n->type = ostk[otop];

    // Prefix unary operators
    if ( n->type == PreInc || n->type == PreDec
    ||   n->type == Plus   || n->type == Minus
    ||   n->type == Inder  || n->type == Deref
    ||   n->type == LogNot || n->type == Not
    ||   n->type == Sizeof )
    {
        if ( ntop < 1 ) mccfail("not enough nodes for pre-unary operator");
        n->right = nstk[--ntop];
        n->left  = NULL;
    }
    // Postfix unary operators
    else if ( n->type == PostInc || n->type == PostDec )
    {
        if ( ntop < 1 ) mccfail("not enough nodes for post-unary operator");
        n->right = NULL;
        n->left  = nstk[--ntop];
    }
    // Ternary operator
    else if ( n->type == Tern )
    {
        if ( ntop < 3 ) mccfail("not enough nodes for ternary operator");
        struct mccnode * resn = sbrk(sizeof(struct mccnode));
        if ( resn == SBRKFAIL ) mccfail("unable to allocate node for ternary operator");

        resn->type = ':';
        resn->right = nstk[--ntop];
        resn->left  = nstk[--ntop];
        n->left     = nstk[--ntop];
        n->right = resn;
    }
    else
    {
        if ( ntop < 2 ) mccfail("not enough nodes for operator");
        n->right = nstk[--ntop];
        n->left  = nstk[--ntop];
    }

    nstk[ntop++] = n;
}

// Build and return an expression tree
struct mccnode * expr(struct mccnsp * curnsp)
{
    otop = ntop = 0;

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
        if ( tk < Number && tk != '"' && tk != ','
        &&   tk != '?'   && tk != ':'
        &&   tk != '('   && tk != ')'
        &&   tk != '['   && tk != ']' ) mode = 2;

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
        else if ( tk == ']' || tk == ')' || tk == ':' || mode == 2 )
        {
            switch (tk)
            {
                case ')': grp = '('; break;
                case ']': grp = '['; break;
                case ':': grp = '?'; break;
            }

            while ( otop && ostk[otop - 1] != grp ) expr_reduce();

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

                    mode = 0;
                }
                else if ( ostk[otop] == '?' )
                {
                    n = nstk[--ntop];
                    while ( otop && getPrec(ostk[otop - 1]) > 1 ) expr_reduce();
                    nstk[ntop++] = n;

                    ostk[otop++] = Tern;

                    mode = 1;
                }
                else mode = 0;

                ntok();
            }
        }
        else
        {
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

            // Remember: '(' and '[' has a prec of -1
            prec = getPrec(tk);
            // Left associative
            if ( prec == 1 || prec == 12 ) while ( otop && getPrec(ostk[otop - 1]) > prec ) expr_reduce();
            // Right associative
            else while ( otop && getPrec(ostk[otop - 1]) >= prec ) expr_reduce();

#ifdef DEBUG_EXPR
            write( 2, "Push:", 5 );
            writeToken( 2, tk );
            write( 2, "\r\n", 2 );
#endif

            ostk[otop++] = tk;

            mode = tk != PostInc && tk != PostDec;

            ntok();

            // TODO Check if this is a cast
            if ( ostk[otop - 1] == '(' )
            {

            }
        }
    }

    if ( !ntop ) mccfail("Empty expression stack");
    else if ( ntop > 1 ) mccfail("Not enough operators in expression");
    return *nstk;
}
