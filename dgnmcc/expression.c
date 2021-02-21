const unsigned int8_t prectbl[] = {
    Ass,     // 0 - Lowest
    Tern,    // 1 (Right associativity)
    LogOr,   // 2
    LogAnd,  // 3
    Or,      // 4
    Xor,     // 5
    And,     // 6
    Eq,      // 7
    Less,    // 8
    Shl,     // 9
    Add,     // 10
    Mul,     // 11
    Sizeof,  // 12 (Right associativity)
    PostInc, // 13 - Highest
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

void reduce()
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

    n->oper = ostk[otop];
    n->flag = 0;
    n->type = NULL;

    // Prefix unary operators
    if ( n->oper == PreInc || n->oper == PreDec
    ||   n->oper == Plus   || n->oper == Minus
    ||   n->oper == Inder  || n->oper == Deref
    ||   n->oper == LogNot || n->oper == Not
    ||   n->oper == Sizeof )
    {
        if ( ntop < 1 ) mccfail("not enough nodes for pre-unary operator");
        n->right = nstk[--ntop];
        n->left  = NULL;
    }
    // Postfix unary operators
    else if ( n->oper == PostInc || n->oper == PostDec )
    {
        if ( ntop < 1 ) mccfail("not enough nodes for post-unary operator");
        n->right = NULL;
        n->left  = nstk[--ntop];
    }
    // Ternary operator
    else if ( n->oper == Tern )
    {
        if ( ntop < 3 ) mccfail("not enough nodes for ternary operator");
        struct mccnode * resn = sbrk(sizeof(struct mccnode));
        if ( resn == SBRKFAIL ) mccfail("unable to allocate node for ternary operator");

        resn->oper = ':';
        resn->flag = 0;
        resn->type = NULL;
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
    struct mccnode * root, * n;
    unsigned int16_t unary = 1;

    otop = ntop = 0;

    // Build expression tree
    while ( tk >= Number || tk == '"'
    ||      tk == ','    || tk == ';'
    ||      tk == '?'    || tk == ':'
    ||      tk == '('    || tk == ')'
    ||      tk == '['    || tk == ']' )
    {
        int8_t grp = 0;
        unsigned int8_t prec;

        if ( tk == ',' || tk == ';' )
        {
            // Collapse stack
            while ( otop ) reduce();

            if ( tk == ';' ) break; // Done with this expression

            ostk[otop++] = ',';

            unary = 1;

            ntok();
        }
        else if ( tk == Named || tk == Number || tk == '"' || tk == SmolNumber || tk == LongNumber )
        {
            nstk[ntop] = sbrk(sizeof(struct mccnode));
            if ( nstk[ntop] == SBRKFAIL ) mccfail("unable to allocate space for new node");

            nstk[ntop]->left = nstk[ntop]->right = NULL;
            nstk[ntop]->oper = tk;
            nstk[ntop]->flag = 0;
            nstk[ntop]->type = &type_int;
            nstk[ntop]->val = tkVal;

            if ( tk == Named )
            {
                // Attempt to look up this symbol
                struct mccsym * nsym = findSymbol( curnsp, tkStr, tkVal );

                if ( nsym )
                {
                    nstk[ntop]->flag |= CPL_LVAL; // All named variables are l-values
                    nstk[ntop]->oper = Variable;
                    nstk[ntop]->type = &nsym->type;
                    nstk[ntop]->sym  = nsym;

                    ntok();
                    if ( tk == '(' ) // Function call
                    {
                        // TODO
                    }
                }
                else // Probably struct or union member
                {
                    nstk[ntop]->name = sbrk(tkVal);
                    if ( nstk[ntop]->name == SBRKFAIL ) mccfail("unable to allocate space for node name");
                    nstk[ntop]->type = NULL; // No type

                    int16_t i;
                    for ( i = 0; i < tkVal; i++ ) nstk[ntop]->name[i] = tkStr[i];

                    ntok();
                }
            }
            else
            {
                if ( tk == SmolNumber )
                {
                    nstk[ntop]->type = &type_char;
                }
                else if ( tk == LongNumber )
                {
                    nstk[ntop]->valLong = tkLong;
                    nstk[ntop]->type = &type_long;
                }
                else if ( tk == '"' )
                {
                    nstk[ntop]->flag |= CPL_LVAL; // String constants are l-values
                }

                ntok();
            }

            unary = 0;
            ntop++;
        }
        else if ( tk == ']' || tk == ')' || tk == ':' )
        {
            switch (tk)
            {
                case ')': grp = '('; break;
                case ']': grp = '['; break;
                case ':': grp = '?'; break;
            }

            while ( otop && ostk[otop - 1] != grp ) reduce();

            if ( !otop ) mccfail("no matching open parenthasis or bracket in expression");

            otop--;

            if ( ostk[otop] == '[' )
            {
                n = sbrk(sizeof(struct mccnode));
                if ( n == SBRKFAIL ) mccfail("unable to allocate array node");
                n->oper = '[';
                n->flag = 0;

                if ( ntop < 2 ) mccfail("not enough nodes for array operator");
                n->right = nstk[--ntop];
                n->left  = nstk[--ntop];
                nstk[ntop++] = n;

                unary = 0;
            }
            else if ( ostk[otop] == '?' )
            {
                n = nstk[--ntop];
                while ( otop && getPrec(ostk[otop - 1]) > 1 ) reduce();
                nstk[ntop++] = n;

                ostk[otop++] = Tern;

                unary = 1;
            }
            else unary = 0;

            ntok();
        }
        else
        {
            // Convert to unary operators as needed
            if (unary)
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
            if ( prec == 1 || prec == 12 ) while ( otop && getPrec(ostk[otop - 1]) > prec ) reduce();
            // Right associative
            else while ( otop && getPrec(ostk[otop - 1]) >= prec ) reduce();

#ifdef DEBUG_EXPR
            write( 2, "Push:", 5 );
            writeToken( 2, tk );
            write( 2, "\r\n", 2 );
#endif

            ostk[otop++] = tk;

            unary = tk != PostInc && tk != PostDec;

            ntok();

            // TODO Check if this is a cast
            if ( ostk[otop - 1] == '(' )
            {

            }
        }

        // Check for overflows
        if ( otop > MAX_EXPR_OPER ) mccfail( "operator stack full" );
        if ( ntop > MAX_EXPR_NODE ) mccfail( "node stack full" );
    }

    if ( !ntop ) mccfail("Empty expression stack");
    else if ( ntop > 1 ) mccfail("Not enough operators in expression");

    root = n = *nstk;

    dumpTree( root, "expr_tree.dot" );

    // Preform post order traversal for type propogation and constant reduction
    ntop = 0;
    while ( n || ntop )
    {
        while (n)
        {
            if ( n->right ) nstk[ntop++] = n->right;
            nstk[ntop++] = n;

            n = n->left;
        }

        n = nstk[--ntop];

        if ( n->right && nstk[ntop - 1] == n->right )
        {
            ntop--;
            nstk[ntop++] = n;
            n = n->right;
            continue;
        }

/*
*** Supported operators for rvalue types ***
char, uchar, int, uint, long, ulong:
    Ass, AddAss, SubAss, MulAss, DivAss, ModAss, ShlAss, ShrAss, AndAss, XorAss, OrAss,
    Tern,
    LogOr,
    LogAnd,
    Or,
    Xor,
    And,
    Eq, Neq,
    Less, LessEq, Great, GreatEq,
    Shl, Shr,
    Add, Sub,
    Mul, Div, Mod,
    Sizeof, LogNot, Not, Plus, Minus
float, double: (No modulus or binary ops)
    Ass, AddAss, SubAss, MulAss, DivAss,
    Tern,
    LogOr,
    LogAnd,
    Eq, Neq,
    Less, LessEq, Great, GreatEq,
    Add, Sub,
    Mul, Div,
    Sizeof, LogNot, Plus, Minus

*** Supported operators for lvalue types (in addition to those for rvalues) ***
char, uchar, int, uint, long, ulong:
    Inder, PreInc, PreDec,
    PostInc, PostDec
float, double:
    Inder, PreInc, PreDec,
    PostInc, PostDec
struct, union:
    Ass,
    Sizeof, Inder, Dot

*** Supoorted operators for lvalue pointers ***
void, char, uchar, int, uint, long, ulong, float, double:
    Ass, AddAss, SubAss,
    LogOr,
    LogAnd,
    Add, Sub,
    Sizeof, LogNot, Inder, Deref, PreInc, PreDec,
    Array, PostInc, PostDec
struct, union: (In addition to pointers above)
    Arrow

*** DON'T FORGET ABOUT CASTING
*/
        // Typecheck TODO: optimize this
        if ( n->oper >= Ass && n->oper <= OrAss )
        {
            if ( n->oper == Ass ) // Any compatible type
            {
                if ( !isCompatible( n->left->type, n->right->type ) ) mccfail("incompatible types");
            }
            else if ( n->oper >= SubAss ) // Arithmetic Type
            {
                if ( !isArith(n->left->type ) ) mccfail("lhs is not an arithmetic type");
                if ( !isArith(n->right->type) ) mccfail("rhs is not an arithmetic type");
            }
            else if ( n->oper >= ModAss ) // Integer Type
            {
                if ( !isInteger(n->left->type ) ) mccfail("lhs is not an integer type");
                if ( !isInteger(n->right->type) ) mccfail("rhs is not an integer type");
            }
            else // Scalar type (AddAss, SubAss)
            {
                if ( !isScalar(n->left->type ) ) mccfail("lhs is not a scalar type");
                if ( !isScalar(n->right->type) ) mccfail("rhs is not a scalar type");
            }
        }
        else if ( n->oper == Tern )
        {
            if ( !isScalar(n->left->type) ) mccfail("lhs is not a scalar type");
        }
        else if ( n->oper == ':' )
        {
            if ( !isCompatible( n->left->type, n->right->type ) ) mccfail("incompatible types");
        }
        else if ( n->oper == LogOr || n->oper == LogAnd )
        {
            if ( !isScalar(n->left->type ) ) mccfail("lhs is not a scalar type");
            if ( !isScalar(n->right->type) ) mccfail("rhs is not a scalar type");
        }
        else if ( n->oper >= Or && n->oper <= And )
        {
            if ( !isInteger(n->left->type ) ) mccfail("lhs is not an integer type");
            if ( !isInteger(n->right->type) ) mccfail("rhs is not an integer type");
        }
        else if ( n->oper >= Eq && n->oper <= GreatEq )
        {
            if ( !isScalar(n->left->type ) ) mccfail("lhs is not a scalar type");
            if ( !isScalar(n->right->type) ) mccfail("rhs is not a scalar type");
        }
        else if ( n->oper == Shl || n->oper == Shr )
        {
            if ( !isInteger(n->left->type ) ) mccfail("lhs is not an integer type");
            if ( !isInteger(n->right->type) ) mccfail("rhs is not an integer type");
        }
        else if ( n->oper == Add || n->oper == Sub )
        {
            if ( !isScalar(n->left->type ) ) mccfail("lhs is not a scalar type");
            if ( !isScalar(n->right->type) ) mccfail("rhs is not a scalar type");
        }
        else if ( n->oper == Mul || n->oper == Div )
        {
            if ( !isArith(n->left->type ) ) mccfail("lhs is not an arithmetic type");
            if ( !isArith(n->right->type) ) mccfail("rhs is not an arithmetic type");
        }
        else if ( n->oper == Mod )
        {
            if ( !isInteger(n->left->type ) ) mccfail("lhs is not an integer type");
            if ( !isInteger(n->right->type) ) mccfail("rhs is not an integer type");
        }
        else if ( n->oper == Sizeof )
        {
            if ( isFunction(n->right->type) ) mccfail("cannot get sizeof function");
        }
        else if ( n->oper == LogNot )
        {
            if ( !isScalar(n->right->type) ) mccfail("not a scalar type");
        }
        else if ( n->oper == Not )
        {
            if ( !isInteger(n->right->type) ) mccfail("not an integer type");
        }
        else if ( n->oper == Plus || n->oper == Minus )
        {
            if ( !isArith(n->right->type) ) mccfail("not an arithmetic type");
        }
        else if ( n->oper == Inder )
        {
            if ( !isPointer(n->right->type) ) mccfail("not a pointer");
        }
        else if ( n->oper == Deref )
        {
            if ( ~n->right->flag & CPL_LVAL ) mccfail("not an l-value");
        }
        else if ( n->oper == PreInc || n->oper == PreDec )
        {
            if ( ~n->right->flag & CPL_LVAL ) mccfail("not an l-value");
            if ( !isScalar(n->right->type) ) mccfail("not a scalar type");
        }
        else if ( n->oper == PostInc || n->oper == PostDec )
        {
            if ( ~n->left->flag & CPL_LVAL ) mccfail("not an l-value");
            if ( !isScalar(n->left->type) ) mccfail("not a scalar type");
        }
        else if ( n->oper == Dot )
        {
            if ( isPointer(n->left->type) ) mccfail("lhs is a pointer");
            if ( !isStruct(n->left->type) ) mccfail("lhs is not a struct");
        }
        else if ( n->oper == Arrow )
        {
            if ( !isPointer(n->left->type) ) mccfail("lhs is not a pointer");
            if ( !isStruct(n->left->type ) ) mccfail("lhs is not a pointer to a struct");
        }
        else if ( n->oper == '[' )
        {
            if ( !isPointer(n->left->type) ) mccfail("lhs is not a pointer");
            if ( !isInteger(n->right->type) ) mccfail("rhs is not an integer type");
        }


        // Always integer
        if ( n->oper == LogOr || n->oper == LogAnd || n->oper == LogNot )
        {
            if ( n->left->type->stype || n->right->type->stype ) mccfail("incompatible type, struct or union");

            n->type = &type_int;
        }
        // Assignment (Always inherit the left type)
        else if ( n->oper >= Ass && n->oper <= OrAss )
        {
            n->type = n->left->type;
        }
        // Implicit arithmetic conversions
        else if ( n->oper >= Or && n->oper <= Mod || n->oper == ':' )
        {
            // Requires a minimum rank of signed int
            if ( n->oper == Shl || n->oper == Shr || n->oper >= Not && n->oper <= Minus )
            {

            }
        }
        #ifdef DEBUG_EXPR
        else mccfail("no promotion rule for operator");
        #endif

        n = NULL;
    }

    return root;
}
