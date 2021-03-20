struct mccnsp castnsp = { NULL, 0, CPL_CAST, 0, NULL, &castnsp.symtbl, NULL, &castnsp.nsptbl };

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

// Must be declared before stacks to prevent overflow nonsense
unsigned int16_t otop, ntop, ctop;

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
    if ( ostk[otop] == '(' || ostk[otop] == '[' || ostk[otop] == '?' )
        mccfail("wrong matched parenthasis or bracket in expression");

    struct mccnode * n = sbrk(sizeof(struct mccnode));
    if ( n == SBRKFAIL ) mccfail("unable to allocate expression node");

    n->oper = ostk[otop];
    n->flag = 0;
    n->type = NULL;

    // Unary operators
    if ( n->oper == PostInc || n->oper == PostDec
    ||   n->oper == PreInc  || n->oper == PreDec
    ||   n->oper == Plus    || n->oper == Minus
    ||   n->oper == Inder   || n->oper == Deref
    ||   n->oper == LogNot  || n->oper == Not
    ||   n->oper == Sizeof  || n->oper == Cast )
    {
        if ( ntop < 1 ) mccfail("not enough nodes for unary operator");
        n->right = nstk[--ntop];
        n->left  = NULL;

        if ( n->oper == Cast )
        {
            if (!ctop) mccfail("no type for cast");
            n->type = cstk[--ctop];
        }
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
// stk = stop token
struct mccnode * expr(struct mccnsp * curnsp, int8_t stk)
{
    struct mccnode * root, * n;
    unsigned int16_t unary = 1;

    otop = ntop = ctop = 0;

    // Build expression tree
    while ( tk >= Named || tk == '"'
    ||      tk == ','   || tk == ';'
    ||      tk == '?'   || tk == ':'
    ||      tk == '('   || tk == ')'
    ||      tk == '['   || tk == ']' )
    {
        int8_t grp = 0;
        unsigned int8_t prec;

        if ( tk == ',' || tk == ';' )
        {
            // Collapse stack
            while ( otop ) reduce();

            if ( tk == ';' || stk == tk ) break; // Done with this expression

            ostk[otop++] = ',';

            unary = 1;

            ntok();
        }
        else if ( tk == Named || (tk >= Number && tk <= DblNumber) || tk == '"' )
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
                        // TODO function calls
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
                    nstk[ntop]->type = &type_string;
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

            if ( !otop )
            {
                if ( stk == tk ) break;
                mccfail("no matching open parenthasis or bracket in expression");
            }

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

            if ( ostk[otop - 1] == '(' && tk >= Void && tk <= Unsigned ) // Check if cast
            {
                ostk[otop - 1] = Cast;

                castnsp.parent = curnsp;
                define(&castnsp);

                if ( tk != ')' ) mccfail("expected closing parenthasis in cast");
                ntok();

                cstk[ctop++] = &castnsp.symtbl->type;

                // Reset casting namespace
                castnsp.symtbl = NULL;
                castnsp.symtail = &castnsp.symtbl;

                castnsp.size = 0;
            }
        }

        // Check for overflows
        if ( otop > MAX_EXPR_OPER ) mccfail( "operator stack full" );
        if ( ntop > MAX_EXPR_NODE ) mccfail( "node stack full" );
        if ( ctop > MAX_EXPR_CAST ) mccfail( "cast stack full" );
    }

    if ( !ntop ) mccfail("Empty expression stack");
    else if ( ntop > 1 ) mccfail("Not enough operators in expression");

    root = n = *nstk;

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

#if DEBUG_EXPR
        write(2, "Tok: ", 5);
        writeToken(2, n->oper);
        write(2, "\n", 1);
#endif

        // TODO optimize typechecking/promotion
        if ( n->oper >= Ass && n->oper <= OrAss )
        {
            if ( ~n->left->flag & CPL_LVAL ) mccfail("lhs of assignment is not an l-value");

            if ( n->oper == Ass ) // Any compatible type
            {
                if ( isArray(n->left->type) ) mccfail("lhs of assignment is array");
                if ( !isCompatible( n->left->type, n->right->type ) ) mccfail("incompatible types");
            }
            else if ( n->oper == AddAss )
            {
                if ( isPointer(n->left->type) && !isInteger(n->right->type) ) mccfail("lhs is pointer, rhs is not integer type");
                if ( isPointer(n->right->type) && !isInteger(n->left->type) ) mccfail("rhs is pointer, lhs is not integer type");

                if ( !isArith(n->left->type ) ) mccfail("lhs is not an arithmetic type");
                if ( !isArith(n->right->type) ) mccfail("rhs is not an arithmetic type");
            }
            else if ( n->oper == SubAss )
            {
                if ( isPointer(n->left->type) && !isPointer(n->right->type) && !isInteger(n->right->type) ) mccfail("lhs is pointer, rhs is neither pointer nor integer");

                if ( !isArith(n->left->type ) ) mccfail("lhs is not an arithmetic type");
                if ( !isArith(n->right->type) ) mccfail("rhs is not an arithmetic type");
            }
            else if ( n->oper > SubAss ) // Arithmetic Type
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

            n->type = n->left->type; // Always produces a non l-value
        }
        else if ( n->oper == Tern )
        {
            if ( !isScalar(n->left->type) ) mccfail("lhs is not a scalar type");
            n->type = n->right->type;
            n->flag = n->right->flag;
        }
        else if ( n->oper == ':' ) // Always results in an r-value
        {
            if ( !isCompatible( n->left->type, n->right->type ) ) mccfail("incompatible types");
            // Promote if needed
            if ( isArith(n->left->type) ) n->type = typePromote( n->left->type, n->right->type );
            else n->type = n->left->type;
        }
        else if ( n->oper == LogOr || n->oper == LogAnd )
        {
            if ( !isScalar(n->left->type ) ) mccfail("lhs is not a scalar type");
            if ( !isScalar(n->right->type) ) mccfail("rhs is not a scalar type");
            n->type = &type_int; // Always int
        }
        else if ( n->oper >= Or && n->oper <= And )
        {
            if ( !isInteger(n->left->type ) ) mccfail("lhs is not an integer type");
            if ( !isInteger(n->right->type) ) mccfail("rhs is not an integer type");
            n->type = typePromote(n->left->type, n->right->type);
        }
        else if ( n->oper >= Eq && n->oper <= GreatEq )
        {
            if ( !isScalar(n->left->type ) ) mccfail("lhs is not a scalar type");
            if ( !isScalar(n->right->type) ) mccfail("rhs is not a scalar type");
            n->type = &type_int;
        }
        else if ( n->oper == Shl || n->oper == Shr )
        {
            if ( !isInteger(n->left->type ) ) mccfail("lhs is not an integer type");
            if ( !isInteger(n->right->type) ) mccfail("rhs is not an integer type");
            n->type = typePromote(n->left->type, n->right->type);
            if ( (n->type->ptype & CPL_DTYPE_MASK) < CPL_INT ) n->type = &type_int;
        }
        else if ( n->oper == Add )
        {
            if ( isPointer(n->left->type) )
            {
                if ( !isInteger(n->right->type) ) mccfail("lhs is pointer, rhs is not integer type");
                n->type = n->left->type;
                n->flag |= CPL_LVAL;
            }
            else if ( isPointer(n->right->type) )
            {
                if ( !isInteger(n->left->type) ) mccfail("rhs is pointer, lhs is not integer type");
                n->type = n->right->type;
                n->flag |= CPL_LVAL;
            }
            else
            {
                if ( !isArith(n->left->type ) ) mccfail("lhs is not a arithmetic type");
                if ( !isArith(n->right->type) ) mccfail("rhs is not a arithmetic type");
                n->type = typePromote(n->left->type, n->right->type);
            }
        }
        else if ( n->oper == Sub )
        {
            if ( isPointer(n->left->type) )
            {
                if ( isPointer(n->right->type) ) n->type = &type_int;
                else if ( isInteger(n->right->type) ) n->type = n->left->type, n->flag = CPL_LVAL;
                else mccfail("lhs is pointer, rhs is neither pointer nor integer");
            }
            else
            {
                if ( !isArith(n->left->type ) ) mccfail("lhs is not a arithmetic type");
                if ( !isArith(n->right->type) ) mccfail("rhs is not a arithmetic type");
                n->type = typePromote(n->left->type, n->right->type);
            }
        }
        else if ( n->oper == Mul || n->oper == Div )
        {
            if ( !isArith(n->left->type ) ) mccfail("lhs is not an arithmetic type");
            if ( !isArith(n->right->type) ) mccfail("rhs is not an arithmetic type");
            n->type = typePromote(n->left->type, n->right->type);
        }
        else if ( n->oper == Mod )
        {
            if ( !isInteger(n->left->type ) ) mccfail("lhs is not an integer type");
            if ( !isInteger(n->right->type) ) mccfail("rhs is not an integer type");
            n->type = typePromote(n->left->type, n->right->type);
        }
        else if ( n->oper == Sizeof )
        {
            if ( isFunction(n->right->type) ) mccfail("cannot get sizeof function");
            n->type = &type_int;
        }
        else if ( n->oper == LogNot )
        {
            if ( !isScalar(n->right->type) ) mccfail("not a scalar type");
            n->type = &type_int;
        }
        else if ( n->oper == Not )
        {
            if ( !isInteger(n->right->type) ) mccfail("not an integer type");
            n->type = n->right->type;
            if ( (n->type->ptype & CPL_DTYPE_MASK) < CPL_INT ) n->type = &type_int;
        }
        else if ( n->oper == Plus || n->oper == Minus )
        {
            if ( !isArith(n->right->type) ) mccfail("not an arithmetic type");
            n->type = n->right->type;
            if ( (n->type->ptype & CPL_DTYPE_MASK) < CPL_INT ) n->type = &type_int;
        }
        else if ( n->oper == Inder )
        {
            if ( ~n->right->flag & CPL_LVAL ) mccfail("not an l-value");
            n->type = typeInder(n->right->type);
        }
        else if ( n->oper == Deref )
        {
            if ( !isPointer(n->right->type) ) mccfail("not a pointer");
            n->type = typeDeref(n->right->type);

            n->flag |= CPL_LVAL;
        }
        else if ( n->oper == PreInc || n->oper == PreDec || n->oper == PostInc || n->oper == PostDec )
        {
            if ( ~n->right->flag & CPL_LVAL ) mccfail("not an l-value");
            if ( !isScalar(n->right->type) ) mccfail("not a scalar type");
            n->type = n->right->type;

            n->flag |= CPL_LVAL;
        }
        else if ( n->oper == Dot )
        {
            if ( !isStruct(n->left->type) ) mccfail("lhs is not a struct");
            if ( isPointer(n->left->type) ) mccfail("lhs is a pointer");
            // TODO lookup type of rhs member

            n->flag |= CPL_LVAL;
        }
        else if ( n->oper == Arrow )
        {
//            if ( !isPointer(n->left->type) ) mccfail("lhs is not a pointer");
            if ( !isStruct(n->left->type ) ) mccfail("lhs is not a struct type");

            struct mccsubtype * s = n->left->type->sub;
            if ( s->sub || s->inder + s->arrays - 1 ) mccfail("lhs not a pointer to struct");

            // TODO lookup type of rhs member

            n->flag |= CPL_LVAL;
        }
        else if ( n->oper == '[' )
        {
            if ( !isPointer(n->left->type) ) mccfail("lhs is not a pointer");
            if ( !isInteger(n->right->type) ) mccfail("rhs is not an integer type");
            n->type = typeDeref(n->left->type);
            n->flag |= CPL_LVAL;
        }
        else if ( n->oper == Cast )
        {
            if ( !isScalar(n->right->type) ) mccfail("rhs of cast is not a scalar");

            // Type of cast is pre-defined

            // Never results in an l-value
        }
        else if ( n->oper == ',' ) n->type = n->right->type, n->flag = n->right->flag; // No rules for ,
#ifdef DEBUG_EXPR
        else mccfail("no typecheck rule for operator");

        if (!n->type) mccfail("no promotion rule for operator");
#endif

        if ( n->oper == Sizeof ) // Always results in a constant expr
        {
            n->oper = Number;
            n->val = typeSize(n->right->type);
            n->right->type = NULL;
        }
        else if ( n->oper == Tern )
        {
            // TODO constant expr ternary
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
                    case LogOr:   vl = vl || vr; break;
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
#ifdef DEBUG_EXPR
                    default: mccfail("unknown constant expr operator");
#endif
                }

                // Promote to integer if needed
                if (n->left->oper == Number || n->right->oper == Number
                     // Implicit promotion to integer
                     || n->oper == Shl || n->oper == Shr ) n->oper = Number, n->val = vl;
                else n->oper = SmolNumber, n->val = (int8_t)vl;

                n->left = n->right = NULL;
            }
            else if ( !n->left ) // Unary operator
            {
                int16_t vd = 1, vl = n->left->val;

                switch (n->oper)
                {
                    case Plus:   vl = +vl; break;
                    case Minus:  vl = -vl; break;
                    case Not:    vl = ~vl; break;
                    case LogNot: vl = !vl; break;
                    default: vd = 0; // Not all unary operators can be resolved like casts, example: a = *12;
                }

                if ( vd )
                {
                    // All valid unary operations result in integer promotion
                    n->oper = Number;
                    n->val = vl;
                    n->right = NULL;
                }
            }
        }

        n = NULL;
    }

#if DEBUG_EXPR
    dumpTree( root, "type_tree.dot" );
#endif

    return root;
}

void emit(struct mccnsp * curnsp, struct mccnode * root)
{
    struct mccnode * n = root;

    // Preform post order traversal for code emission
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

        

        n = NULL;
    }
}
