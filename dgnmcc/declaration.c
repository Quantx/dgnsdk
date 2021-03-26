unsigned int16_t svid = 1; // Static variable ID (starts at 1)

void declare( struct mccnsp * curnsp, struct mccsym * cursym, struct mccsubtype ** partype )
{
    struct mccsubtype * curtype = *partype;

    // Initialize type
    curtype->inder = 0;
    curtype->arrays = 0;

    curtype->sizes = NULL;

    curtype->ftype = NULL;
    curtype->sub = NULL;

    // Get number of indirects
    while ( tk == Mul ) { curtype->inder++; ntok(); }

    // Get any subtypes
    if ( tk == '(' )
    {
        ntok();

        curtype->sub = sbrk( sizeof(struct mccsubtype) );
        if ( curtype->sub == SBRKFAIL ) mccfail("unable to allocate space for subtype");

        declare( curnsp, cursym, &curtype->sub );

        if ( tk != ')' ) mccfail("expected closing parenthasis 0");
    }
    // Get symbol name
    else if ( tk != Named ) mccfail("expected declaration name");
    else
    {
        // Allocate room for symbol name
        cursym->name = sbrk( tkVal );
        if ( cursym->name == SBRKFAIL ) mccfail("unable to allocate space for symbol name");

        // Copy symbol name
        int16_t i;
        for ( i = 0; i < tkVal; i++ ) cursym->name[i] = tkStr[i];
        cursym->len = tkVal;
    }

    ntok();

    // Pointer sizes to end of current program break
    // Do this here in case we pass arrays up
    curtype->sizes = sbrk(0);

    // Get array declarations
    if ( tk == '[' )
    {
        while ( tk == '[' )
        {
            ntok();

            unsigned int16_t * cas = sbrk( sizeof(unsigned int16_t) );
            if ( cas == SBRKFAIL ) mccfail("unable to allocate space for array size");

            void * erbp = sbrk(0);

            struct mccnode * cexp = expr(curnsp, ']');

            if ( tk != ']' ) mccfail("expected closing bracket");
            ntok();

            // Don't support dynamic array sizes because that would require dynamic typing
            if ( cexp->oper < Number || cexp->oper > LongNumber ) mccfail("need constant integer expression in array declaration");
            *cas = cexp->val;

            brk(erbp); // Free expression stack

            curtype->arrays++;
        }
    }
    else if ( tk == '(' ) // Get function declarations
    {
        struct mccnsp * fncnsp = curtype->ftype = sbrk( sizeof(struct mccnsp) );
        if ( fncnsp == SBRKFAIL ) mccfail("unable to allocate space for new namespace");

        fncnsp->name = NULL;
        fncnsp->len = 0;

        fncnsp->type = CPL_FUNC;

        fncnsp->addr = 0;
        fncnsp->size = 0;

        fncnsp->symtbl = NULL;
        fncnsp->nsptbl = NULL;

        fncnsp->symtail = &fncnsp->symtbl;
        fncnsp->nsptail = &fncnsp->nsptbl;

        fncnsp->parent = &glbnsp;
        fncnsp->next = NULL;

        ntok();
        while ( tk != ')' )
        {
            // This is a variadic function
            if ( tk == Variadic )
            {
                fncnsp->type = CPL_VFUNC;

                ntok();
                if ( tk != ')' ) mccfail("expected closing parenthasis 1");
                break;
            }

            define( fncnsp );

            if ( tk == Comma ) ntok();
            else if ( tk != ')' ) mccfail("expected closing parenthasis 2");
        }

        ntok();
    }

    if ( curtype->sub )
    {
        // Pass up arrays if possible
        if ( !(curtype->sub->inder || curtype->sub->ftype) ) curtype->sub->arrays += curtype->arrays, curtype->arrays = 0;

        // Pass up inderections if possible
        if ( !(curtype->arrays || curtype->ftype) ) curtype->sub->inder += curtype->inder, curtype->inder = 0;
    }

    // Fold child through empty subtypes
    if ((
            // This is the base subtype, it's allowed to be empty
            (cursym->type.sub == *partype && curtype->sub)
            // This is not the base subtype
            || cursym->type.sub != *partype
        )
        // Check if empty
        && !curtype->inder
        && !curtype->arrays
        && !curtype->ftype ) *partype = curtype->sub;
}

void outputVarAddr( int16_t segfd, struct mccnode * root )
{
    struct mccnode * n;
    for ( n = root; n && n->oper == Dot; n = n->left )
    {
        decwrite( segfd, n->right->val );
        write( segfd, " + ", 3 );
    }

    if ( !( n && n->oper == Variable ) ) mccfail("cannot output address of non-immediate-lvalue");

    write( segfd, n->sym->name, n->sym->len );
    if ( n->sym->addr )
    {
        write( segfd, ".", 1 );
        decwrite( segfd, n->sym->addr );
    }
}

void instantiate( struct mccnsp * curnsp, int16_t segfd, struct mcctype * curtype )
{
    struct mccsubtype * s;
    for ( s = curtype->sub; s->sub; s = s->sub );

    if ( s->ftype ) mccfail("cannot instantiate a function");

    if ( s->arrays )
    {
        if ( tk != '{' ) mccfail("expected opening curly-brace");
        ntok();

        struct mcctype * dt = typeDeref(curtype);

        unsigned int16_t i = 0;
        while ( i < *s->sizes )
        {
            instantiate(curnsp, segfd, dt);

            i++;
            if ( tk == '}' ) break;
            if ( tk == Comma ) ntok();
            else mccfail("expected closing curly-brace or comma");
        }

        if ( tk != '}' ) mccfail("expected closing curly-brace");
        ntok();

        // Output filler if needed
        if ( i = *s->sizes - i )
        {
            // Fill rest of array with zeros
            write( segfd, "\t.zero ", 7 );
            decwrite( segfd, i * typeSize(dt) );
            write( segfd, "\n", 1 );
        }

        brk(dt);

        return;
    }

    if ( curtype->stype ) // TODO union declaration
    {
        if ( tk != '{' ) mccfail("expected opening curly-brace");
        ntok();

        int16_t isz = 0;
        struct mccsym * cursym;
        struct mccnsp * subnsp;
        for ( cursym = curtype->stype->symtbl, subnsp = curtype->stype->nsptbl; cursym; cursym = cursym->next )
        {
            // Find next relevant anonymous child namespace
            while ( subnsp
                && ( !subnsp->symtbl || (!subnsp->name && (~subnsp->type & CPL_INST)))
            ) subnsp = subnsp->next;

            if ( subnsp && subnsp->addr < cursym->addr )
            {
                if ( isz < subnsp->addr ) // Mid struct padding
                {
                    write( segfd, "\t.zero ", 7 );
                    decwrite( segfd, isz );
                    write( segfd, "\n", 1 );
                }

                struct mcctype nspt;
                nspt.stype = subnsp;
                nspt.sub = sbrk( sizeof(struct mccsubtype) );
                if ( nspt.sub == SBRKFAIL ) mccfail("unable to allocate space for instantiation anonymous sub namespace");
                nspt.sub->inder = 0;
                nspt.sub->arrays = 0;
                nspt.sub->ftype = NULL;
                nspt.sub->sub = NULL;

                instantiate( curnsp, segfd, &nspt );

                brk(nspt.sub);

                isz += subnsp->size;
                subnsp = subnsp->next;
            }
            else
            {
                if ( isz < subnsp->addr ) // Mid struct padding
                {
                    write( segfd, "\t.zero ", 7 );
                    decwrite( segfd, isz );
                    write( segfd, "\n", 1 );
                }

                instantiate( curnsp, segfd, &cursym->type );
                isz += typeSize( &cursym->type );
            }

            if ( tk == '}' ) break;
            if ( tk == Comma ) ntok();
            else mccfail("expected closing curly-brace or comma");
        }

        if ( tk != '}' ) mccfail("expected closing curly-brace");
        ntok();

        // End of struct padding
        if ( isz = curtype->stype->size - isz )
        {
            // Fill rest of struct with zeros
            write( segfd, "\t.zero ", 7 );
            decwrite( segfd, isz );
            write( segfd, "\n", 1 );
        }

        return;
    }

    void * erbp = sbrk(0);

    struct mccnode * cexp = expr(curnsp, Comma);

    int16_t comp = isCompatible( curtype, cexp->type );

    if ( !comp ) mccfail("incompatible initialization type");
    else if ( comp < 0 ) mccfail("constant to large for type");

    if ( cexp->oper >= Number && cexp->oper <= DblNumber ) // Integer expression
    {
        switch ( typeSize(curtype) )
        {
            case 1: write( segfd, "\t.byte ", 7 ); break;
            case 2: write( segfd, "\t.word ", 7 ); break;
            case 4: write( segfd, "\t.long ", 7 ); break;
            case 8: write( segfd, "\t.quad ", 7 ); break;
        }

        // TODO-FPU output floats
        if ( cexp->oper == LongNumber ) octwrite( segfd, cexp->valLong );
        else decwrite( segfd, cexp->val );
    }
    // String constant
    else if ( cexp->oper == '"' )
    {
        write( segfd, "\t~SC", 4 );
        decwrite( segfd, cexp->val );
        write( segfd, "\n", 1 );
    }
    // Address-of (Inderection) operations
    else if ( cexp->oper == Inder )
    {
        write( segfd, "\t", 1 );
        outputVarAddr( segfd, cexp->right );
        write( segfd, "\n", 1 );
    }
    else if ( cexp->oper == Sub )
    {
        if ( cexp->left->oper != Inder ) mccfail("lhs of const subtraction is not address of");
        if ( cexp->right->oper < SmolNumber || cexp->right->oper > LongNumber ) mccfail("rhs of const subtraction is not number");

        write( segfd, "\t", 1 );
        outputVarAddr( segfd, cexp->left );
        write( segfd, " - ", 3 );
        decwrite( segfd, cexp->right->val );
        write( segfd, "\n", 1 );
    }
    else if ( cexp->oper == Add )
    {
        struct mccnode * aofn;
        int16_t av;

        if ( cexp->left->oper == Inder )
        {
            if ( cexp->right->oper < SmolNumber || cexp->right->oper > LongNumber ) mccfail("rhs of address-of addtion is not number");
            aofn = cexp->left;
            av = cexp->right->val;
        }
        else if ( cexp->right->oper == Inder )
        {
            if ( cexp->left->oper < SmolNumber || cexp->left->oper > LongNumber ) mccfail("lhs of address-of addtion is not number");
            aofn = cexp->right;
            av = cexp->left->val;
        }
        else mccfail("not a constant expression");

        write( segfd, "\t", 1 );
        outputVarAddr( segfd, aofn );
        write( segfd, " + ", 3 );
        decwrite( segfd, av );
        write( segfd, "\n", 1 );
    }
    else mccfail("non-constant initializer");

    write( segfd, "\n", 1 );

    brk(erbp);
}

// (extern) (const|register|static) (signed|unsigned) <void|char|int|...|struct|union|...|etc>
void define( struct mccnsp * curnsp )
{
    #define LNK_EXTERN 1
    #define LNK_STATIC 2

    unsigned int8_t ctype = 0, cnsp = 0, lnk = 0;

    unsigned int8_t nsptype = curnsp->type & CPL_NSPACE_MASK;

    if ( nsptype == CPL_BLOCK )
    {
        if ( tk == Extern ) lnk = LNK_EXTERN, ntok();

        // Check storage type
        switch ( tk ) // Check for storage type
        {
            case Const:    ctype = CPL_CNST; ntok(); break;
            case Register: ctype = CPL_ZERO; ntok(); break;
            case Static:
                if ( lnk ) mccfail("variable cannot be extern and static");
                lnk = LNK_STATIC;
                ctype = CPL_BSS;
                ntok();
                break;
            case Auto:
                ntok();
            default:
                if ( curnsp == &glbnsp ) ctype = CPL_BSS;
                else if ( lnk ) mccfail("non file scope variable cannot be extern");
                else ctype = CPL_STAK;
        }
    }
    else ctype |= CPL_STAK;

    if ( tk == Unsigned )
    {
        ntok();
        switch ( tk )
        {
            case Char: ctype |= CPL_UCHR; break;
            case Int:  ctype |= CPL_UINT; break;
            case Long: ctype |= CPL_ULNG; break;
            default: mccfail("invalid unsigned type in declaration");
        }
    }
    else if ( tk == Signed )
    {
        ntok();
        switch ( tk )
        {
            case Char: ctype |= CPL_CHR; break;
            case Int:  ctype |= CPL_INT; break;
            case Long: ctype |= CPL_LNG; break;
            default: mccfail("invalid signed type in declaration");
        }
    }
    else switch ( tk )
    {
        // Primative datatypes
        case Void:   ctype |= CPL_VOID; break;
        case Char:   ctype |= CPL_CHR;  break;
        case Int:    ctype |= CPL_INT;  break;
        case Long:   ctype |= CPL_LNG;  break;
        case Float:  ctype |= CPL_FPV;  break;
        // Structures
        case Struct: cnsp  |= CPL_STRC; break;
        case Enum:   cnsp  |= CPL_ENUM; break;
        case Union:  cnsp  |= CPL_UNION; break;

        default: mccfail("missing type in declaration");
    }

    ntok();

    struct mccnsp * decnsp = NULL;

    // Get struct name or anonymous definition
    if ( cnsp & CPL_NSPACE_MASK )
    {
        if ( !(tk == Named || tk == '{') ) mccfail("expected struct name or anonymous struct definition");

        if ( tk == Named )
        {
            decnsp = findNamespace( curnsp, tkStr, tkVal );
            if ( decnsp ) ntok();
        }

        if ( !decnsp || tk == '{' ) // Generate new struct
        {
            struct mccnsp * newnsp = sbrk( sizeof(struct mccnsp) );
            if ( newnsp == SBRKFAIL ) mccfail("unable to allocate space for new struct");

            if ( decnsp ) // Override existing struct
            {
                // Copy name
                newnsp->name = sbrk( decnsp->len );
                if ( newnsp->name == SBRKFAIL ) mccfail("unable to allocate space for new struct name");

                int16_t i;
                for ( i = 0; i < decnsp->len; i++ ) newnsp->name[i] = decnsp->name[i];
                newnsp->len = decnsp->len;
            }
            else if ( tk == Named ) // Creating a new struct for the first time
            {
                // Copy name
                newnsp->name = sbrk( tkVal );
                if ( newnsp->name == SBRKFAIL ) mccfail("unable to allocate space for new struct name");

                int16_t i;
                for ( i = 0; i < tkVal; i++ ) newnsp->name[i] = tkStr[i];
                newnsp->len = tkVal;
            }
            else // Anonymous struct
            {
                newnsp->name = NULL;
                newnsp->len = 0;
            }

            // Record namespace type
            newnsp->type = cnsp;

            // Init tables
            newnsp->size = 0;
            newnsp->symtbl = NULL;
            newnsp->nsptbl = NULL;

            newnsp->symtail = &newnsp->symtbl;
            newnsp->nsptail = &newnsp->nsptbl;

            // Add to parent namespace
            newnsp->parent = curnsp;
            newnsp->next = NULL;

            *curnsp->nsptail = newnsp;
            curnsp->nsptail = &newnsp->next;

            decnsp = newnsp;
            if ( tk != '{' ) ntok();
        }
        // Type check existing struct
        else if ( (decnsp->type & CPL_NSPACE_MASK) != (cnsp & CPL_NSPACE_MASK) ) mccfail("struct type missmatch");

        // Struct definition
        if ( tk == '{' )
        {
            // Don't redfine things
            if ( decnsp->type & CPL_DEFN ) mccfail("struct redefinition");
            decnsp->type |= CPL_DEFN;

            // Anonymous struct, inherit parent offset
            if ( !decnsp->name ) decnsp->addr = curnsp->size;

            if ( cnsp == CPL_ENUM )
            {
                while ( tk != '}' )
                {
                    ntok();
                    if ( tk != Named ) mccfail("expected named symbol");
                }
            }
            else
            {
                ntok(); // Drop opening {
                while ( tk != '}' )
                {
                    define(decnsp);
                }
            }

            // Anonymous struct, update parent size
            if ( !decnsp->name ) curnsp->size += decnsp->size;

            ntok(); // Discard last
        }
    }

    while ( tk != ';' )
    {
        /*
        Allocate new symbol

        Used for type checking with an existing symbol
        It will be deallocated later if it already exists
        */

        struct mccsym * cursym = sbrk( sizeof(struct mccsym) );
        if ( cursym == SBRKFAIL ) mccfail( "unable to allocate space for new symbol" );

        cursym->name = NULL;
        cursym->len = 0;

        cursym->type.ptype = ctype; // Record primative datatype
        cursym->type.stype = decnsp; // Record if this is a struct or not

        if ( decnsp ) decnsp->type |= CPL_INST; // Mark this struct as instantiated

        cursym->addr = 0;

        cursym->next = NULL;

        cursym->type.sub = sbrk( sizeof(struct mccsubtype) );
        if ( cursym->type.sub == SBRKFAIL ) mccfail( "unable to allocate space for new symbol subtype" );

        // Process type information
        declare( curnsp, cursym, &cursym->type.sub );

        if ( nsptype != CPL_CAST // You can cast to VOID
          && !( (ctype & CPL_DTYPE_MASK) || (cnsp & CPL_NSPACE_MASK) )
          && !(  cursym->type.sub->inder ||  cursym->type.sub->ftype ) ) mccfail("can't declare variable storing void");

        // You can declare either a pointer to a function returning the struct, or a pointer to the struct itself
        if ( decnsp == curnsp
          && !( cursym->type.sub->inder || cursym->type.sub->ftype ) ) mccfail("recursive struct declaration");

        // You can only declare pointers to incomplete types
        if ( decnsp && (~decnsp->type & CPL_DEFN)
          && !( cursym->type.sub->inder || cursym->type.sub->ftype ) ) mccfail("cannot declare incomplete type");

        struct mccsym * chksym = getSymbol( curnsp, cursym->name, cursym->len );

        if ( chksym )
        {
            // Types must be EXACT, not just compatible
            struct mcctype * ta, * tb;
            struct mccsubtype * sa, * sb;

            ta = &cursym->type;
            tb = &chksym->type;

            if ( ta->stype != tb->stype ) mccfail("incompatible struct types in redeclaration");
            if ( !ta->stype && (ta->ptype & CPL_DTYPE_MASK) != (tb->ptype & CPL_DTYPE_MASK) ) mccfail("incompatible primative types in redeclaration");

            for ( sa = ta->sub, sb = tb->sub; sa && sb; sa = sa->sub, sb = sb->sub )
            {
                if ( sa->inder != sb->inder ) mccfail("inderection missmatch in redeclaration");
                if ( sa->arrays != sb->arrays ) mccfail("array dimension missmatch in redeclaration");

                int16_t i;
                for ( i = 0; i < sa->arrays; i++ ) if ( sa->sizes[i] != sb->sizes[i] ) mccfail("array size missmatch in redeclaration");

                if ( sa->ftype )
                {
                    if ( !sb->ftype ) mccfail("incompatible function types in redeclaration");

                    if ( (sa->ftype->type & CPL_NSPACE_MASK) != (sb->ftype->type & CPL_NSPACE_MASK) ) mccfail("variadic function incompatible with non-variadic function in redeclaration");

                    struct mccsym * asym, * bsym;
                    for ( asym = sa->ftype->symtbl, bsym = sb->ftype->symtbl; asym && bsym; asym = asym->next, bsym = bsym->next )
                    {
                        if ( !isCompatible( &asym->type, &bsym->type ) ) mccfail("incompatible argument types in redeclaration");
                    }

                    if ( asym || bsym ) mccfail("incompatile number of arguments in redeclaration");
                }
            }

            if ( sa || sb ) mccfail("type missmatch in redeclaration");

            // Unallocate current symbol and start using old one
            brk(cursym); cursym = chksym;

            if ( (cursym->type.ptype & CPL_LOCAL) && lnk != LNK_EXTERN ) mccfail("variable has already been declared");
        }
        // Add new symbol to current namespace
        else *curnsp->symtail = cursym, curnsp->symtail = &cursym->next;

        // Get innermost subtype
        struct mccsubtype * s;
        for ( s = cursym->type.sub; s->sub; s = s->sub );

        if ( s->ftype )
        {
            if ( curnsp != &glbnsp ) mccfail("function declared outside file scope");

            // Function with codeblock
            if ( tk == '{' )
            {
                // Mark function as defined
                if ( s->ftype->type & CPL_DEFN ) mccfail("function already declared");
                s->ftype->type |= CPL_DEFN;

                cursym->type.ptype &= ~CPL_STORE_MASK;
                cursym->type.ptype |= CPL_TEXT | CPL_LOCAL;

                if ( lnk == LNK_STATIC ) emitStatement( Label, cursym->len );
                else emitStatement( LabelExtern, cursym->len );

                write( segs[SEG_TEXT], cursym->name, cursym->len );

                statement(cursym, s->ftype, 0);
            }

            break; // Remember: execution stops here for functions
        }

        if ( (cursym->type.ptype & CPL_STORE_MASK) == CPL_STAK ) // Stack variable declaration
        {
            // Compute address
            unsigned int16_t tsz = typeSize(&cursym->type);

            if ( nsptype == CPL_UNION ) curnsp->size = tsz > curnsp->size ? tsz : curnsp->size;
            else
            {
                if ( curnsp->size & 1 && tsz > 1 ) curnsp->size++; // Align to word if needed
                cursym->addr = curnsp->size;
                curnsp->size += tsz;
            }

            if ( nsptype == CPL_BLOCK ) emitStatement( Allocate, tsz );

            cursym->type.ptype |= CPL_LOCAL;
        }
        else if ( lnk == LNK_EXTERN ) // Not declaring anything here, just marking as global
        {
            write( segs[SEG_DATA], "\t.glob ", 7 );
            write( segs[SEG_DATA], cursym->name, cursym->len );
            write( segs[SEG_DATA], "\n", 1 );
        }
        else // Data or BSS variable declaration
        {
            // No longer initialized to zero so convert BSS to DATA
            if ( tk == Ass && (cursym->type.ptype & CPL_STORE_MASK) == CPL_BSS )
            {
                cursym->type.ptype &= ~CPL_STORE_MASK;
                cursym->type.ptype |= CPL_DATA;
            }

            int16_t segfd = segs[(cursym->type.ptype & CPL_STORE_MASK) >> 4];

            write( segfd, cursym->name, cursym->len );

            // A block scope static variable is being declared
            if ( curnsp != &glbnsp )
            {
                write( segfd, ".", 1 );
                decwrite( segfd, cursym->addr = svid++ );
            }

            write( segfd, ":\n", 2 );

            if ( tk == Ass )
            {
                ntok();
                instantiate(curnsp, segfd, &cursym->type);
            }
            else
            {
                write( segfd, "\t.zero ", 7 );
                decwrite( segfd, typeSize(&cursym->type) );
                write( segfd, "\n", 1 );
            }

            cursym->type.ptype |= CPL_LOCAL;
        }

        // Only declare one variable per definition
        if ( nsptype == CPL_CAST
          || nsptype == CPL_FUNC
          || nsptype == CPL_VFUNC ) break;
        if ( tk == Comma ) ntok();
        else if ( tk != ';' ) mccfail("expected comma or semi-colon");
    }

    if ( tk == ';' ) ntok();
}
