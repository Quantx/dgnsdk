unsigned int16_t svid; // Static variable ID (starts at 1)

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
    if ( tk == Paren_L )
    {
        ntok();

        curtype->sub = sbrk( sizeof(struct mccsubtype CAST_NAME) );
        if ( curtype->sub == SBRKFAIL ) mccfail("unable to allocate space for subtype");

        declare( curnsp, cursym, &curtype->sub );

        if ( tk != Paren_R ) mccfail("expected closing parenthasis 0");
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
    if ( tk == Square_L )
    {
        while ( tk == Square_L )
        {
            ntok();

            unsigned int16_t * cas = sbrk( sizeof(unsigned int16_t CAST_NAME) );
            if ( cas == SBRKFAIL ) mccfail("unable to allocate space for array size");

            void * erbp = sbrk(0);

            struct mccnode * cexp = expr(curnsp, Square_R);

            if ( tk != Square_R ) mccfail("expected closing bracket");
            ntok();

            // Don't support dynamic array sizes because that would require dynamic typing
            if ( cexp->oper < Number || cexp->oper > SmolNumber ) mccfail("need constant integer expression in array declaration");
            *cas = cexp->val;

            brk(erbp); // Free expression stack

            curtype->arrays++;
        }
    }
    else if ( tk == Paren_L ) // Get function declarations
    {
        struct mccnsp * fncnsp = curtype->ftype = sbrk( sizeof(struct mccnsp CAST_NAME) );
        if ( fncnsp == SBRKFAIL ) mccfail("unable to allocate space for new namespace");

        fncnsp->name = NULL;
        fncnsp->len = 0;

        fncnsp->type = CPL_FUNC;

        fncnsp->addr = 0;
        fncnsp->size = 0;

        fncnsp->symtbl = NULL; fncnsp->symtail = &fncnsp->symtbl;
        fncnsp->nsptbl = NULL; fncnsp->nsptail = &fncnsp->nsptbl;

        fncnsp->parent = curnsp;
        fncnsp->next = NULL;

        ntok();
        while ( tk != Paren_R )
        {
            // This is a variadic function
            if ( tk == Variadic )
            {
                fncnsp->type = CPL_VFUNC;

                ntok();
                if ( tk != Paren_R ) mccfail("expected closing parenthasis 1");
                break;
            }

            define( fncnsp );

            if ( tk == Comma ) ntok();
            else if ( tk != Paren_R ) mccfail("expected closing parenthasis 2");
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
        if ( tk != Curly_L ) mccfail("expected opening curly-brace");
        ntok();

        struct mcctype * dt = typeDeref(curtype);

        unsigned int16_t i = 0;
        while ( i < *s->sizes )
        {
            instantiate(curnsp, segfd, dt);

            i++;
            if ( tk == Curly_R ) break;
            if ( tk == Comma ) ntok();
            else mccfail("expected closing curly-brace or comma");
        }

        if ( tk != Curly_R ) mccfail("expected closing curly-brace");
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

    if ( curtype->stype && !s->inder ) // TODO union declaration
    {
        if ( tk != Curly_L ) mccfail("expected opening curly-brace");
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
                    decwrite( segfd, subnsp->addr - isz );
                    write( segfd, "\n", 1 );
                }

                struct mcctype nspt;
                nspt.ptype = CPL_LOCAL | CPL_STAK;
                nspt.stype = subnsp;

                nspt.sub = sbrk( sizeof(struct mccsubtype CAST_NAME) );
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
                if ( isz < cursym->addr ) // Mid struct padding
                {
                    write( segfd, "\t.zero ", 7 );
                    decwrite( segfd, cursym->addr - isz );
                    write( segfd, "\n", 1 );
                }

#ifdef DEBUG_DECLARE
                writeType( 1, &cursym->type, cursym->name, cursym->len );
                write( 1, "\n", 1 );
#endif
                instantiate( curnsp, segfd, &cursym->type );
                isz += typeSize( &cursym->type );
            }

            if ( tk == Curly_R ) break;
            if ( tk == Comma ) ntok();
            else mccfail("expected closing curly-brace or comma");
        }

        if ( tk != Curly_R ) mccfail("expected closing curly-brace");
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
//    else if ( comp < 0 ) mccfail("constant to large for type");

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
    else if ( cexp->oper == String )
    {
        write( segfd, "\t.word ~SC", 10 );
        decwrite( segfd, cexp->val );
    }
    // Address-of (Inderection) operations
    else if ( cexp->oper == Inder )
    {
        write( segfd, "\t.word ", 7 );
        outputVarAddr( segfd, cexp->right );
    }
    else if ( cexp->oper == Sub )
    {
        if ( cexp->left->oper != Inder ) mccfail("lhs of const subtraction is not address of");
        if ( cexp->right->oper < SmolNumber || cexp->right->oper > LongNumber ) mccfail("rhs of const subtraction is not number");

        write( segfd, "\t.word ", 7 );
        outputVarAddr( segfd, cexp->left );
        write( segfd, " - ", 3 );
        decwrite( segfd, cexp->right->val );
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

        write( segfd, "\t.word ", 7 );
        outputVarAddr( segfd, aofn );
        write( segfd, " + ", 3 );
        decwrite( segfd, av );
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
    else ctype = CPL_STAK;

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
        case Struct: cnsp = CPL_STRC; break;
        case Enum:   cnsp = CPL_ENUM; break;
        case Union:  cnsp = CPL_UNION; break;

        default: mccfail("missing type in declaration");
    }

    ntok();

    struct mccnsp * decnsp = NULL;

    // Get struct name or anonymous definition
    if ( cnsp & CPL_NSPACE_MASK )
    {
        if ( !(tk == Named || tk == Curly_L) ) mccfail("expected struct name or anonymous struct definition");

        if ( tk == Named )
        {
            decnsp = findNamespace( curnsp, tkStr, tkVal );
            if ( decnsp ) ntok();
        }

        if ( !decnsp || (tk == Curly_L && !getNamespace( curnsp, decnsp->name, decnsp->len )) ) // Generate new struct
        {
            struct mccnsp * newnsp = sbrk( sizeof(struct mccnsp CAST_NAME) );
            if ( newnsp == SBRKFAIL ) mccfail("unable to allocate space for new struct");

            if ( decnsp ) // Override existing struct
            {
                // Copy name
                newnsp->name = sbrk( newnsp->len = decnsp->len );
                if ( newnsp->name == SBRKFAIL ) mccfail("unable to allocate space for new struct name");

                int16_t i;
                for ( i = 0; i < decnsp->len; i++ ) newnsp->name[i] = decnsp->name[i];

#ifdef DEBUG_DECLARE
                write( 2, "Copied named struct ", 21 );
                write( 2, newnsp->name, newnsp->len );
                write( 2, "\n", 1 );
#endif
            }
            else if ( tk == Named ) // Creating a new struct for the first time
            {
                // Copy name
                newnsp->name = sbrk( newnsp->len = tkVal );
                if ( newnsp->name == SBRKFAIL ) mccfail("unable to allocate space for new struct name");

                int16_t i;
                for ( i = 0; i < tkVal; i++ ) newnsp->name[i] = tkStr[i];

#ifdef DEBUG_DECLARE
                write( 2, "New named struct ", 17 );
                write( 2, newnsp->name, newnsp->len );
                write( 2, "\n", 1 );
#endif
            }
            else // Anonymous struct
            {
                newnsp->name = NULL;
                newnsp->len = 0;
            }

            // Record namespace type
            newnsp->type = cnsp;

            newnsp->addr = 0;
            newnsp->size = 0;

            // Init tables
            newnsp->symtbl = NULL; newnsp->symtail = &newnsp->symtbl;
            newnsp->nsptbl = NULL; newnsp->nsptail = &newnsp->nsptbl;

            // Add to parent namespace
            newnsp->parent = curnsp;
            newnsp->next = NULL;

            *curnsp->nsptail = newnsp;
            curnsp->nsptail = &newnsp->next;

            decnsp = newnsp;
            if ( tk != Curly_L ) ntok();
        }
        // Type check existing struct
        else if ( (decnsp->type & CPL_NSPACE_MASK) != (cnsp & CPL_NSPACE_MASK) ) mccfail("struct type missmatch");

        // Struct definition
        if ( tk == Curly_L )
        {
            // Don't redfine things
            if ( decnsp->type & CPL_DEFN ) mccfail("struct redefinition");
            decnsp->type |= CPL_DEFN;

#ifdef DEBUG_DECLARE
            if ( decnsp->name )
            {
                write( 2, "Declared namespace ", 19 );
                write( 2, decnsp->name, decnsp->len );
                write( 2, "\n", 1 );
            }
            else write( 2, "Declared anon namespace\n", 24 );
#endif

            // Anonymous struct, inherit parent offset
            if ( !decnsp->name ) decnsp->addr = curnsp->size;

            ntok();
            if ( cnsp == CPL_ENUM )
            {
                int16_t enumVal = 0;
                while ( tk != Curly_R )
                {
                    if ( tk != Named ) mccfail("expected named symbol");

                    struct mccsym * newsym = sbrk( sizeof(struct mccsym CAST_NAME) );
                    if ( newsym == SBRKFAIL ) mccfail("unable to allocate space for enum symbol");

                    newsym->name = sbrk( newsym->len = tkVal );
                    int16_t i;
                    for ( i = 0; i < tkVal; i++ ) newsym->name[i] = tkStr[i];

                    newsym->type.ptype = CPL_ENUM_CONST;
                    newsym->type.stype = NULL;

                    newsym->type.sub = sbrk( sizeof(struct mccsubtype CAST_NAME) );
                    if ( newsym->type.sub == SBRKFAIL ) mccfail("unable to allocate space for enum symbol subtype");

                    newsym->type.sub->inder = 0;
                    newsym->type.sub->arrays = 0;
                    newsym->type.sub->ftype = NULL;
                    newsym->type.sub->sub = NULL;

                    newsym->next = NULL;

                    ntok();
                    if ( tk == Ass )
                    {
                        ntok();

                        void * erbp = sbrk(0);

                        struct mccnode * cexp = expr(curnsp, Comma);

                        if ( cexp->oper < Number || cexp->oper > SmolNumber ) mccfail("need constant integer expression in array declaration");
                        enumVal = cexp->val;

                        brk(erbp); // Free expression stack
                    }

                    newsym->addr = enumVal++;

                    *decnsp->symtail = newsym;
                    decnsp->symtail = &newsym->next;

                    if ( tk == Comma ) ntok();
                    else if ( tk != Curly_R ) mccfail("expected closing brace or comma");
                }

                // Switch to the primative ENUM type
                decnsp = NULL;
                cnsp = 0;
                ctype |= CPL_ENUM_CONST;
            }
            else
            {
                while ( tk != Curly_R ) define(decnsp);

                // TODO move undefined namespaces to the global namespace
                struct mccnsp ** chknsp, * udfnsp;
                chknsp = &decnsp->nsptbl;
                while ( udfnsp = *chknsp )
                {
//                    udfnsp = *chknsp;
                    if ( ~udfnsp->type & CPL_DEFN )
                    {
                        *chknsp = udfnsp->next;
                        if ( decnsp->nsptail == &udfnsp->next ) decnsp->nsptail = chknsp;

                        udfnsp->parent = &glbnsp;
                        udfnsp->next = NULL;

                        *glbnsp.nsptail = udfnsp;
                        glbnsp.nsptail = &udfnsp->next;
                    }
                    else chknsp = &udfnsp->next;
                }
            }

            // Anonymous struct, update parent size
            if ( decnsp && !decnsp->name ) curnsp->size += decnsp->size;

            ntok(); // Discard last

#ifdef DEBUG_DECLARE
            dumpNamespace( 2, decnsp );
#endif
        }
    }

    while ( tk != SemiColon )
    {
        /*
        Allocate new symbol

        Used for type checking with an existing symbol
        It will be deallocated later if it already exists
        */

        struct mccsym * cursym = sbrk( sizeof(struct mccsym CAST_NAME) );
        if ( cursym == SBRKFAIL ) mccfail( "unable to allocate space for new symbol" );

        cursym->name = NULL;
        cursym->len = 0;

        cursym->type.ptype = ctype; // Record primative datatype
        cursym->type.stype = decnsp; // Record if this is a struct or not

        if ( decnsp ) decnsp->type |= CPL_INST; // Mark this struct as instantiated

        cursym->addr = 0;

        cursym->next = NULL;

        cursym->type.sub = sbrk( sizeof(struct mccsubtype CAST_NAME) );
        if ( cursym->type.sub == SBRKFAIL ) mccfail( "unable to allocate space for new symbol subtype" );

        // Process type information
        declare( curnsp, cursym, &cursym->type.sub );

        if ( nsptype != CPL_CAST // You can cast to void
          && !( cursym->type.stype || (cursym->type.ptype & CPL_DTYPE_MASK)
             || cursym->type.sub->inder || cursym->type.sub->ftype ) ) mccfail("can't declare variable storing void");

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

        if ( nsptype == CPL_CAST ) return; // We're done casting

        // Get innermost subtype
        struct mccsubtype * s;
        for ( s = cursym->type.sub; s->sub; s = s->sub );

        if ( s->ftype )
        {
            if ( curnsp != &glbnsp ) mccfail("function declared outside file scope");

            // Function with codeblock
            if ( tk == Curly_L )
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

        if ( (cursym->type.ptype & CPL_STORE_MASK) == CPL_STAK ) // Stack variable or struct member declaration
        {
            // Compute address
            unsigned int16_t tsz = typeSize(&cursym->type);
            unsigned int16_t pad = 0;

            if ( nsptype == CPL_UNION ) curnsp->size = tsz > curnsp->size ? tsz : curnsp->size;
            else
            {
                if ( curnsp->size & 1 && tsz > 1 ) curnsp->size += (pad = 1); // Align to word if needed
                cursym->addr = curnsp->size;
                curnsp->size += tsz;
            }

            if ( nsptype == CPL_BLOCK ) // Stack variable
            {
                emitStatement( Allocate, tsz + pad );

                if ( tk == Ass )
                {
                    unary = 0;
                    otop = ctop = 0;
                    expr_no_reset = ntop = 1;

                    void * erbp = sbrk(0);

                    *nstk = sbrk(sizeof(struct mccnode CAST_NAME));
                    if ( *nstk == SBRKFAIL ) mccfail("unable to allocate space for new node");

                    // TODO stack struct constant initialization: struct mystruct a = { 1, 2 };

                    (*nstk)->left = (*nstk)->right = NULL;
                    (*nstk)->oper = Variable;
                    (*nstk)->flag |= CPL_LVAL;
                    (*nstk)->type = &cursym->type;
                    (*nstk)->sym  = cursym;

                    struct mccnode * root = expr(curnsp, Comma);

                    emit(root);

                    brk(erbp);
                }
            }

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
                decwrite( segfd, cursym->addr = ++svid );
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
        else if ( tk != SemiColon ) mccfail("expected comma or semi-colon");
    }

    if ( tk == SemiColon ) ntok();
}
