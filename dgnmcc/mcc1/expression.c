struct mccnsp castnsp = { NULL, 0, CPL_CAST, 0, 0, NULL, &castnsp.symtbl, NULL, &castnsp.nsptbl };

const unsigned int8_t prectbl[15] = {
    Comma,   // 0 - Lowest
    Ass,     // 1 (Right associativity)
    Tern,    // 2 (Right associativity)
    LogOr,   // 3
    LogAnd,  // 4
    Or,      // 5
    Xor,     // 6
    And,     // 7
    Eq,      // 8
    Less,    // 9
    Shl,     // 10
    Add,     // 11
    Mul,     // 12
    Sizeof,  // 13 (Right associativity)
    PostInc, // 14 - Highest
};

int8_t getPrec( unsigned int8_t op )
{
    int8_t i;
    for ( i = 0; i < sizeof(prectbl) && op >= prectbl[i]; i++ );
    return i - 1;
}

// Must be declared before stacks to prevent overflow nonsense
unsigned int16_t otop, ntop, ctop, unary;

unsigned int8_t ostk[MAX_EXPR_OPER];

struct mccnode * nstk[MAX_EXPR_NODE];

struct mcctype * cstk[MAX_EXPR_CAST];

void reduce()
{
    otop--;

#ifdef DEBUG_EXPR
    write( 2, "Pop:", 4 );
    writeToken( 2, ostk[otop] );
    write( 2, "\r\n", 2 );
#endif

    // This should never happen
    if ( ostk[otop] == Paren_L || ostk[otop] == Square_L || ostk[otop] == Q_Mark )
        fail("wrong matched parenthasis or bracket in expression");

    struct mccnode * n = sbrk(sizeof(struct mccnode CAST_NAME));
    if ( n == SBRKFAIL ) fail("unable to allocate expression node");

    n->oper = ostk[otop];
    n->flag = 0;
    n->type = NULL;

    // Unary operators
    if ( n->oper == PostInc || n->oper == PostDec
    ||   n->oper == PreInc  || n->oper == PreDec
    ||   n->oper == Plus    || n->oper == Minus
    ||   n->oper == Inder   || n->oper == Deref
    ||   n->oper == LogNot  || n->oper == Not
    ||   n->oper == Sizeof  || n->oper == Cast
    ||   n->oper == FnCall )
    {
        if ( ntop < 1 ) fail("not enough nodes for unary operator");
        n->right = nstk[--ntop];
        n->left  = NULL;

        if ( n->oper == Cast )
        {
            if (!ctop) fail("no type for cast");
            n->type = cstk[--ctop];
        }
    }
    // Ternary operator
    else if ( n->oper == Tern )
    {
        if ( ntop < 3 ) fail("not enough nodes for ternary operator");
        struct mccnode * resn = sbrk(sizeof(struct mccnode CAST_NAME));
        if ( resn == SBRKFAIL ) fail("unable to allocate node for ternary operator");

        resn->oper = Colon;
        resn->flag = 0;
        resn->type = NULL;
        resn->right = nstk[--ntop];
        resn->left  = nstk[--ntop];
        n->left     = nstk[--ntop];
        n->right = resn;
    }
    else
    {
        if ( ntop < 2 ) fail("not enough nodes for operator");
        n->right = nstk[--ntop];
        n->left  = nstk[--ntop];
    }

    nstk[ntop++] = n;
}

void makeCast(struct mccnsp * curnsp)
{
    castnsp.parent = curnsp;
    define(&castnsp);

    if ( tk != Paren_R ) fail("expected closing parenthasis in cast");
    ntok();

    cstk[ctop++] = &castnsp.symtbl->type;

    // Reset casting namespace
    castnsp.symtbl = NULL;
    castnsp.symtail = &castnsp.symtbl;
    castnsp.size = 0;
}

int16_t expr_no_reset;

// Build and return an expression tree
// stk = stop token
struct mccnode * expr(struct mccnsp * curnsp, unsigned int8_t stk)
{
/*
#ifdef DEBUG_EXPR
    write( 1, "*** EXPR ", 9 );
    decwrite( 1, ln );
    write( 1, ":", 1 );
    decwrite( 1, p - pp );
    write( 1, "\n", 1 );
#endif
*/
    struct mccnode * root, * n;

    if ( expr_no_reset ) expr_no_reset = 0;
    else
    {
        unary = 1;
        otop = ntop = ctop = 0;
    }

    // Build expression tree
    while ( tk >= Named
    ||      tk == String
    ||      tk == Q_Mark   || tk == Colon
    ||      tk == Paren_L  || tk == Paren_R
    ||      tk == Square_L || tk == Square_R )
    {
        int8_t grp = 0;
        int8_t prec;

        if ( tk == Named || (tk >= Number && tk <= DblNumber) || tk == String )
        {
            nstk[ntop] = sbrk(sizeof(struct mccnode CAST_NAME));
            if ( nstk[ntop] == SBRKFAIL ) fail("unable to allocate space for new node");

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
                    // Check if this is an Enum constant
                    if ( !nsym->type.stype && (nsym->type.ptype & CPL_DTYPE_MASK) == CPL_ENUM_CONST )
                    {
                        nstk[ntop]->oper = Number;
                        nstk[ntop]->val  = nsym->addr;
                    }
                    else
                    {
                        nstk[ntop]->flag |= CPL_LVAL; // All named variables are l-values
                        nstk[ntop]->oper = Variable;
                        nstk[ntop]->sym  = nsym;
                    }
                    nstk[ntop]->type = &nsym->type;
                }
                else // Probably struct or union member
                {
                    nstk[ntop]->name = sbrk(tkVal);
                    if ( nstk[ntop]->name == SBRKFAIL ) fail("unable to allocate space for node name");
                    nstk[ntop]->type = NULL; // No type

                    int16_t i;
                    for ( i = 0; i < tkVal; i++ ) nstk[ntop]->name[i] = tkStr[i];
                }
            }
            else if ( tk == SmolNumber )
            {
                nstk[ntop]->type = &type_char;
            }
            else if ( tk == LongNumber )
            {
                nstk[ntop]->valLong = tkLong;
                nstk[ntop]->type = &type_long;
            }
            else if ( tk == String )
            {
                //nstk[ntop]->flag |= CPL_LVAL; // String constants are NOT l-values
                nstk[ntop]->type = &type_string;
            }

            ntok();

            unary = 0;
            ntop++;
        }
        else if ( tk == Square_R || tk == Paren_R || tk == Colon )
        {
            switch (tk)
            {
                case Paren_R: grp = Paren_L; break;
                case Square_R: grp = Square_L; break;
                case Colon: grp = Q_Mark; break;
            }

            while ( otop && ostk[otop - 1] != grp ) reduce();

            if ( !otop )
            {
                if ( stk == tk ) break;
                fail("no matching open parenthasis or bracket in expression");
            }

            otop--;

            if ( ostk[otop] == Square_L )
            {
                n = sbrk(sizeof(struct mccnode CAST_NAME));
                if ( n == SBRKFAIL ) fail("unable to allocate array node");
                n->oper = Square_L;
                n->flag = 0;
                n->type = NULL;

                if ( ntop < 2 ) fail("not enough nodes for array operator");
                n->right = nstk[--ntop];
                n->left  = nstk[--ntop];
                nstk[ntop++] = n;

                unary = 0;
            }
            else if ( ostk[otop] == Q_Mark )
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
        else if ( tk == stk && tk != Comma ) break;
        else
        {
            if ( tk == stk )
            {
                // Don't break for comma if we're currently in a function declaration
                int16_t i;
                for ( i = 0; i < otop && ostk[i] != FnCallArgs; i++ );
                if ( i == otop ) break;
            }

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
            else if (tk == Paren_L) tk = FnCallArgs;

            unsigned int8_t otk = tk;
            ntok();

            if ( otk == FnCallArgs && tk == Paren_R ) otk = FnCall, ntok(); // No args in function call
            else if ( otk == Paren_L && tk >= Void && tk <= Unsigned )
            {
                otk = Cast; // Check if cast
                if ( ostk[otop - 1] == Sizeof )
                {
                    otop--;

                    makeCast(curnsp);

                    nstk[ntop] = sbrk(sizeof(struct mccnode CAST_NAME));
                    if ( nstk[ntop] == SBRKFAIL ) fail("unable to allocate space for new sizeof node");

                    nstk[ntop]->oper = Sizeof;
                    nstk[ntop]->flag = 0;
                    nstk[ntop]->type = NULL;
                    nstk[ntop]->left = NULL;

                    struct mccnode * cn = nstk[ntop++]->right = sbrk(sizeof(struct mccnode CAST_NAME));
                    if ( cn == SBRKFAIL ) fail("unable to allocate space for new sizeof-cast node");

                    cn->oper = Cast;
                    cn->flag = 0;
                    cn->type = cstk[--ctop];
                    cn->left = cn->right = NULL;

                    unary = 0;
                    otk = 0;
                }
            }

            if ( otk )
            {
                // Remember: "(" and ")" has a prec of -1 everywhere except here
                if ( otk == Paren_L || otk == Q_Mark ) prec = 127;
                else if ( otk == Square_L ) prec = 14;
                else prec = getPrec(otk);

                // Left associative
                if ( prec == 1 || prec == 2 || prec == 13 ) while ( otop && getPrec(ostk[otop - 1]) > prec ) reduce();
                // Right associative
                else while ( otop && getPrec(ostk[otop - 1]) >= prec ) reduce();

#ifdef DEBUG_EXPR
                write( 2, "Push:", 5 );
                writeToken( 2, otk );
                write( 2, ":", 1 );
                decwrite( 2, prec );
                write( 2, "\n", 1 );
#endif

                ostk[otop++] = otk;

                if ( otk == FnCallArgs ) ostk[otop++] = Paren_L, unary = 1;
                else unary = otk != PostInc && otk != PostDec;

                if ( otk == Cast ) makeCast(curnsp); // Process casting type
            }
        }

        // Check for overflows
        if ( otop > MAX_EXPR_OPER ) fail( "operator stack full" );
        if ( ntop > MAX_EXPR_NODE ) fail( "node stack full" );
        if ( ctop > MAX_EXPR_CAST ) fail( "cast stack full" );
    }

    while ( otop ) reduce(); // Final reduction

    if ( !ntop ) fail("Empty expression stack");
    else if ( ntop > 1 ) fail("Not enough operators in expression");

    root = n = *nstk;

    char path[32];
    unsigned int32_t ctns;
    unsigned int16_t path_pos;
    int16_t i;
    
#if DEBUG_EXPR
    char * expr_path = "tree/expr0000000000.dot";
    for (i = 0; expr_path[i]; i++) {
        path[i] = expr_path[i];
    }
    path[i] = '\0';
    ctns = ctn;
    path_pos = 18;
    while (ctns) {
        path[path_pos] = (ctns & 7) + '0';
        path_pos--;
        ctns >>= 3;
    }

    dumpTree( root, path );
#endif


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
            nstk[ntop - 1] = n;
            n = n->right;
            continue;
        }

        if ( !(n->left||n->right) ) { n = NULL; continue; } // Ignore leafs
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

*** Supported operators for lvalue pointers ***
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

#if DEBUG_TYPECHECK
        write(2, "Type tok: ", 9);
        writeToken(2, n->oper);
        write(2, "\n", 1);
#endif

        // TODO-OPTIMIZE typechecking/promotion
        if ( n->oper >= Ass && n->oper <= OrAss )
        {
            if ( ~n->left->flag & CPL_LVAL ) fail("lhs of assignment is not an l-value");

            if ( n->oper == Ass ) // Any compatible type
            {
                if ( isArray(n->left->type) ) fail("lhs of assignment is array");
                if ( !isCompatible( n->left->type, n->right->type ) ) fail("incompatible types");
            }
            else if ( n->oper == AddAss )
            {
                if      ( isPointer(n->left->type)  ) if ( !isInteger(n->right->type) ) fail("lhs is pointer, rhs is not integer type");
                else if ( isPointer(n->right->type) ) if ( !isInteger(n->left->type)  ) fail("rhs is pointer, lhs is not integer type");
                else
                {
                    if ( !isArith(n->left->type ) ) fail("lhs is not an arithmetic type");
                    if ( !isArith(n->right->type) ) fail("rhs is not an arithmetic type");
                }
            }
            else if ( n->oper == SubAss )
            {
                if ( isPointer(n->left->type) && !isPointer(n->right->type) && !isInteger(n->right->type) ) fail("lhs is pointer, rhs is neither pointer nor integer");

                if ( !isArith(n->left->type ) ) fail("lhs is not an arithmetic type");
                if ( !isArith(n->right->type) ) fail("rhs is not an arithmetic type");
            }
            else if ( n->oper > SubAss ) // Arithmetic Type
            {
                if ( !isArith(n->left->type ) ) fail("lhs is not an arithmetic type");
                if ( !isArith(n->right->type) ) fail("rhs is not an arithmetic type");
            }
            else if ( n->oper >= ModAss ) // Integer Type
            {
                if ( !isInteger(n->left->type ) ) fail("lhs is not an integer type");
                if ( !isInteger(n->right->type) ) fail("rhs is not an integer type");
            }
            else // Scalar type (AddAss, SubAss)
            {
                if ( !isScalar(n->left->type ) ) fail("lhs is not a scalar type");
                if ( !isScalar(n->right->type) ) fail("rhs is not a scalar type");
            }

            n->type = n->left->type;
        }
        else if ( n->oper == Tern )
        {
            if ( !isScalar(n->left->type) ) fail("lhs is not a scalar type");
            n->type = n->right->type;
            n->flag = n->right->flag;
        }
        else if ( n->oper == Colon ) // Always results in an r-value
        {
            if ( !isCompatible( n->left->type, n->right->type ) ) fail("incompatible types");
            // Promote if needed
            if ( isArith(n->left->type) ) n->type = typePromote( n->left->type, n->right->type );
            else n->type = n->left->type;
        }
        else if ( n->oper == LogOr || n->oper == LogAnd )
        {
            if ( !isScalar(n->left->type ) ) fail("lhs is not a scalar type");
            if ( !isScalar(n->right->type) ) fail("rhs is not a scalar type");
            n->type = &type_int; // Always int
        }
        else if ( n->oper >= Or && n->oper <= And )
        {
            if ( !isInteger(n->left->type ) ) fail("lhs is not an integer type");
            if ( !isInteger(n->right->type) ) fail("rhs is not an integer type");
            n->type = typePromote(n->left->type, n->right->type);
        }
        else if ( n->oper >= Eq && n->oper <= GreatEq )
        {
            if ( !isScalar(n->left->type ) ) fail("lhs is not a scalar type");
            if ( !isScalar(n->right->type) ) fail("rhs is not a scalar type");
            n->type = &type_int;
        }
        else if ( n->oper == Shl || n->oper == Shr )
        {
            if ( !isInteger(n->left->type ) ) fail("lhs is not an integer type");
            if ( !isInteger(n->right->type) ) fail("rhs is not an integer type");
            n->type = typePromote(n->left->type, n->right->type);
            if ( (n->type->ptype & CPL_DTYPE_MASK) < CPL_INT ) n->type = &type_int;
        }
        else if ( n->oper == Add )
        {
            if ( isPointer(n->left->type) )
            {
                if ( !isInteger(n->right->type) ) fail("lhs is pointer, rhs is not integer type");
                n->type = n->left->type;
//                n->flag |= CPL_LVAL;
            }
            else if ( isPointer(n->right->type) )
            {
                if ( !isInteger(n->left->type) ) fail("rhs is pointer, lhs is not integer type");
                n->type = n->right->type;
//                n->flag |= CPL_LVAL;
            }
            else
            {
                if ( !isArith(n->left->type ) ) fail("lhs is not a arithmetic type");
                if ( !isArith(n->right->type) ) fail("rhs is not a arithmetic type");
                n->type = typePromote(n->left->type, n->right->type);
            }
        }
        else if ( n->oper == Sub )
        {
            if ( isPointer(n->left->type) )
            {
                if ( isPointer(n->right->type) ) n->type = &type_int;
                else if ( isInteger(n->right->type) ) n->type = n->left->type;//, n->flag = CPL_LVAL;
                else fail("lhs is pointer, rhs is neither pointer nor integer");
            }
            else
            {
                if ( !isArith(n->left->type ) ) fail("lhs is not a arithmetic type");
                if ( !isArith(n->right->type) ) fail("rhs is not a arithmetic type");
                n->type = typePromote(n->left->type, n->right->type);
            }
        }
        else if ( n->oper == Mul || n->oper == Div )
        {
            if ( !isArith(n->left->type ) ) fail("lhs is not an arithmetic type");
            if ( !isArith(n->right->type) ) fail("rhs is not an arithmetic type");
            n->type = typePromote(n->left->type, n->right->type);
        }
        else if ( n->oper == Mod )
        {
            if ( !isInteger(n->left->type ) ) fail("lhs is not an integer type");
            if ( !isInteger(n->right->type) ) fail("rhs is not an integer type");
            n->type = typePromote(n->left->type, n->right->type);
        }
        else if ( n->oper == Sizeof )
        {
            if ( isFunction(n->right->type) ) fail("sizeof can not be used on a function");
            n->type = &type_int;
        }
        else if ( n->oper == LogNot )
        {
            if ( !isScalar(n->right->type) ) fail("not a scalar type");
            n->type = &type_int;
        }
        else if ( n->oper == Not )
        {
            if ( !isInteger(n->right->type) ) fail("not an integer type");
            n->type = n->right->type;
            if ( (n->type->ptype & CPL_DTYPE_MASK) < CPL_INT ) n->type = &type_int;
        }
        else if ( n->oper == Plus || n->oper == Minus )
        {
            if ( !isArith(n->right->type) ) fail("not an arithmetic type");
            n->type = n->right->type;
            if ( (n->type->ptype & CPL_DTYPE_MASK) < CPL_INT ) n->type = &type_int;
        }
        else if ( n->oper == Inder )
        {
            if ( ~n->right->flag & CPL_LVAL ) fail("not an l-value");
            n->type = typeInder(n->right->type);
        }
        else if ( n->oper == Deref )
        {
            if ( !isPointer(n->right->type) ) fail("not a pointer");
            n->type = typeDeref(n->right->type);

            n->flag |= CPL_LVAL;
        }
        else if ( n->oper == PreInc || n->oper == PreDec || n->oper == PostInc || n->oper == PostDec )
        {
            if ( ~n->right->flag & CPL_LVAL ) fail("not an l-value");
            if ( !isScalar(n->right->type) ) fail("not a scalar type");
            n->type = n->right->type;

            // According to cppreference.com, these operators never result in an l-value
        }
        else if ( n->oper == Dot )
        {
            if ( !isStruct(n->left->type) ) fail("lhs is not a struct");
            if ( isPointer(n->left->type) ) fail("lhs is a pointer");

            int8_t * name = n->right->name;
            int16_t len = n->right->val;

            if ( n->right->oper == Variable ) name = n->right->sym->name, len = n->right->sym->len;
            else if ( n->right->oper != Named ) fail("rhs is not named symbol");

            struct mccsym * ms = getSymbol( n->left->type->stype, name, len );

//            dumpNamespace( 2, n->left->type->stype );
            if ( !ms ) fail("rhs is not a member of lhs");

            n->type = &ms->type;
            n->flag |= CPL_LVAL;

            n->right->oper = Number;
            n->right->type = &type_int;
            n->right->val = ms->addr;
            n->right->flag = 0;
        }
        else if ( n->oper == Arrow )
        {
            if ( !isStruct(n->left->type ) ) fail("lhs is not a struct type");
            if ( !isPointer(n->left->type) ) fail("lhs is not a pointer");

            struct mccsubtype * s = n->left->type->sub;
            if ( s->sub || s->inder + s->arrays != 1 ) fail("lhs not a pointer to struct");

            int8_t * name = n->right->name;
            int16_t len = n->right->val;

            if ( n->right->oper == Variable ) name = n->right->sym->name, len = n->right->sym->len;
            else if ( n->right->oper != Named ) fail("rhs is not named symbol");

            struct mccsym * ms = getSymbol( n->left->type->stype, name, len );

            if ( !ms ) fail("rhs is not a member of lhs");

            n->type = &ms->type;
            n->flag |= CPL_LVAL;

            n->right->oper = Number;
            n->right->type = &type_int;
            n->right->val = ms->addr;
            n->right->flag = 0;
        }
        else if ( n->oper == Square_L )
        {
            if ( !isPointer(n->left->type) ) fail("lhs is not a pointer");
            if ( !isInteger(n->right->type) ) fail("rhs is not an integer type");
            n->type = typeDeref(n->left->type);
            n->flag |= CPL_LVAL;
        }
        else if ( n->oper == Cast )
        {
            if ( n->right && !isScalar(n->right->type) ) fail("rhs of cast is not a scalar");

            // Type of cast is pre-defined

            // Never results in an l-value
        }
        else if ( n->oper == FnCall )
        {
            n->type = typeClone(n->right->type);
            struct mccsubtype * s;
            for ( s = n->type->sub; s->sub; s = s->sub );

            if ( !s->ftype ) fail("lhs is not a function");

            if ( s->ftype->symtbl ) fail("function expects arguments");

            s->ftype = NULL;

            // Functions never return l-values
        }
        else if ( n->oper == FnCallArgs )
        {
            n->type = typeClone(n->left->type);
            struct mccsubtype * s;
            for ( s = n->type->sub; s->sub; s = s->sub );

            if ( !s->ftype ) fail("lhs is not a function");

            struct mccnode * fan, * fsn;
            struct mccsym * cursym;

            // Get first argument, and convert Commas to Args
            for ( fsn = n->right; fsn->left->oper == Comma; fsn = fsn->left ) fsn->oper = Arg;
            fsn->oper = Arg;

            fan = fsn->left;
            // Insert last Arg node
            fsn->left = sbrk(sizeof(struct mccnode CAST_NAME));
            if ( fsn->left == SBRKFAIL ) fail( "unable to allocate room for new argument node");
            fsn = fsn->left;
            
            fsn->oper = Arg;
            fsn->flag = 0;
            fsn->type = fan->type; 
            fsn->right = fan; // Place first arg in correct spot
            
            fsn->left = sbrk(sizeof(struct mccnode CAST_NAME));
            if ( fsn->left == SBRKFAIL ) fail( "unable to allocate room for new StartOfArgs node");
            fsn = fsn->left;
            
            fsn->oper = StartOfArgs;
            fsn->flag = 0;
            fsn->type = n->type;
            fsn->sym = n->sym;
            fsn->left = fsn->right = NULL;
            
            for ( cursym = s->ftype->symtbl; cursym; cursym = cursym->next, fsn = fan )
            {
                for ( fan = n->right; fan->left != fsn; fan = fan->left );

                if ( !isCompatible( &cursym->type, fan->type ) )
                {
#ifdef DEBUG_TYPECHECK
                    write( 2, "Type A: ", 8 );
                    writeType( 2, &cursym->type, cursym->name, cursym->len );
                    write( 2, "\nType B: ", 9 );
                    writeType( 2, fan->type, NULL, 0 );
                    write( 2, "\n", 1 );
#endif
                    fail("incompatible argument");
                }
#ifdef DEBUG_TYPECHECK
                else
                {
                    write( 2, "Args match\n", 11 );
                }
#endif

                if ( fan == n->right ) { cursym = cursym->next; break; }
            }

            if ( cursym ) fail("not enough arguments");
            if ( fan != n->right && (s->ftype->type & CPL_NSPACE_MASK) != CPL_VFUNC ) fail("too many arguments");

            // Just drop the function namespace to get the return type
            s->ftype = NULL;

            // Functions do not return l-values
        }
        else if ( n->oper == Comma ) n->type = n->right->type, n->flag = n->right->flag; // No rules for ,
#ifdef DEBUG_TYPECHECK
        else fail("no typecheck rule for operator");

        if (!n->type) fail("no promotion rule for operator");
#endif

        if ( n->oper == Colon || n->oper == Comma );
        else if ( n->oper == Cast && n->right )
        {
            n->oper = n->right->oper;
            n->flag = n->right->flag;
            // Type stays the same
            n->sym = n->right->sym;
            n->valLong = n->right->valLong;
            n->left = n->right->left;
            n->right = n->right->right;
        }
        else if ( n->oper == Sizeof ) // Always results in a constant expr
        {
            n->oper = Number;
            n->val = typeSize(n->right->type);
            n->right = NULL;
        }
        else if ( n->oper == Tern && (n->left->oper == Number || n->left->oper == SmolNumber ) )
        {
            *n = *(n->left->val ? n->right->left : n->right->right);

            n->flag &= ~CPL_LVAL; // Unset l-value flag since ternary always results in an r-value
        }
        else if ( n->right && (n->right->oper == Number || n->right->oper == SmolNumber) )
        {
            if ( n->left && (n->left->oper == Number || n->left->oper == SmolNumber) )
            {
                int16_t vl, vr;
                vl = n->left->val;
                vr = n->right->val;

                switch (n->oper)
                {
                    case LogOr:   vl = vl || vr; break; // TODO add short circuit behavior
                    case LogAnd:  vl = vl && vr; break;
                    case Or:      vl = vl |  vr; break;
                    case Xor:     vl = vl ^  vr; break;
                    case And:     vl = vl &  vr; break;
                    case Eq:      vl = vl == vr; break;
                    case Neq:     vl = vl != vr; break;
                    case Less:    vl = vl <  vr; break;
                    case LessEq:  vl = vl <= vr; break;
                    case Great:   vl = vl >  vr; break;
                    case GreatEq: vl = vl >= vr; break;
                    case Shl:     vl = vl << vr; break;
                    case Shr:     vl = vl >> vr; break;
                    case Add:     vl = vl +  vr; break;
                    case Sub:     vl = vl -  vr; break;
                    case Mul:     vl = vl *  vr; break;
                    case Div:     vl = vl /  vr; break;
                    case Mod:     vl = vl %  vr; break;
#ifdef DEBUG_TYPECHECK
                    default:
                        writeToken( 2, n->oper );
                        write( 2, "\n", 1 );
                        fail("unknown constant expr operator");
#endif
                }

                // Promote to integer if needed
                if (n->left->oper == Number || n->right->oper == Number
                     // Implicit promotion to integer
                     || n->oper == Shl || n->oper == Shr ) n->oper = Number, n->val = vl;
                else n->oper = SmolNumber, n->val = (int8_t CAST_NAME)vl;

                n->left = n->right = NULL;
            }
            else if ( !n->left ) // Unary operator
            {
                int16_t vd = 1, vr = n->right->val;

                switch (n->oper)
                {
                    case Plus:   vr = +vr; break;
                    case Minus:  vr = -vr; break;
                    case Not:    vr = ~vr; break;
                    case LogNot: vr = !vr; break;
                    default: vd = 0; // Not all unary operators can be resolved like casts, example: a = *12;
                }

                if ( vd )
                {
                    // All valid unary operations result in integer promotion
                    n->oper = Number;
                    n->val = vr;
                    n->right = NULL;
                }
            }
        }

        n = NULL;
    }

#if DEBUG_TYPECHECK
    char * type_path = "tree/type0000000000.dot";
    for (i = 0; type_path[i]; i++) {
        path[i] = type_path[i];
    }
    path[i] = '\0';
    ctns = ctn;
    path_pos = 18;
    while (ctns) {
        path[path_pos] = (ctns & 7) + '0';
        path_pos--;
        ctns >>= 3;
    }

    dumpTree( root, path );
#endif

    return root;
}

void emitStatement(unsigned int8_t op, unsigned int32_t val)
{
    struct mccstmt stn;

#ifdef DEBUG
    if ( op == 0 )
    {
        write( 2, "ZERO STMT\n", 12 );
    }
#endif

    stn.oper = op;
    stn.val = val;
    stn.type = op == SmolNumber ? IR_UCHR : IR_VOID;

    write( segs[SEG_TEXT], &stn, sizeof(struct mccstmt CAST_NAME) );
}

void emitNode(struct mccnode * n)
{
    // Check if this variable is actually on the stack
    if ( n->oper == Variable && (n->sym->type.ptype & CPL_STORE_MASK) == CPL_STAK ) n->oper = VariableLocal, n->val = n->sym->addr;

    struct mccstmt stn;

    stn.oper = n->oper;
    stn.size = 0;

    unsigned int8_t pt = n->type->ptype & CPL_DTYPE_MASK;

    if ( isPointer(n->type) ) stn.type = isArray(n->type) ? IR_ARRAY : IR_PTR;
/*
    {
        stn.type = isArray(n->type) ? IR_ARRAY : IR_PTR;

        struct mcctype * dt = typeDeref(n->type);
        // Handle a void pointer
        if ( !( dt->stype || (dt->ptype & CPL_DTYPE_MASK) || dt->sub->inder || dt->sub->ftype ) ) stn.size = 1;
        else stn.size = typeSize(dt);
        brk(dt);
    }
*/
    else if ( isFunction(n->type) ) stn.size = IR_PTR_SIZE, stn.type = IR_FUNC;
    else if ( isStruct(n->type) )
    {
        stn.type = IR_STRUC;
//        stn.size = typeSize(n->type);
    }
    else if ( pt == CPL_ENUM_CONST ) stn.type = IR_INT;
    else stn.type = pt;

    if ( !stn.size ) stn.size = typeSize(n->type);

    if ( n->flag & CPL_LVAL ) stn.type |= IR_LVAL;

    int8_t tmpbuf[6];
    int16_t tmppos = 6;

    if ( n->oper == Variable ) // Compute variable name length
    {
        stn.name = NULL;
        stn.val = n->sym->len;

        unsigned int16_t addr;

        if ( addr = n->sym->addr )
        {
            while ( addr )
            {
                tmpbuf[--tmppos] = addr % 10 + '0';
                addr /= 10;
            }
            stn.val += 7 - tmppos; // Include the period seperator
        }
    }
    else stn.valLong = n->valLong;

    write( segs[SEG_TEXT], &stn, sizeof(struct mccstmt CAST_NAME) );

    if ( n->oper == Variable ) // Output variable name
    {
        write( segs[SEG_TEXT], n->sym->name, n->sym->len );

        if ( n->sym->addr ) // Output static variable identifier
        {
            write( segs[SEG_TEXT], ".", 1 );
            write( segs[SEG_TEXT], tmpbuf + tmppos, 6 - tmppos );
        }
    }
}

void emit(struct mccnode * n)
{
    //struct mccnode * n = root;

    // Preform pre order traversal for code emission
    *nstk = n;
    ntop = 1;
    while ( ntop )
    {
        n = nstk[--ntop];
    
        emitNode(n);
        
        if ( n->right ) nstk[ntop++] = n->right;
        if ( n->left  ) nstk[ntop++] = n->left;
    }
}

