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

            // MAYBE add code emition for non-file scope arrays
            if ( cexp->oper < Number || cexp->oper > LongNumber ) mccfail("need constant integer expression in array declaration");
            *cas = cexp->val;

            brk(erbp); // Free expression stack

            curtype->arrays++;

            if ( tk != ']' ) mccfail("expected closing bracket");
            ntok();
        }
    }
    else if ( tk == '(' ) // Get function declarations
    {
        struct mccnsp * fncnsp = curtype->ftype = sbrk( sizeof(struct mccnsp) );
        if ( fncnsp == SBRKFAIL ) mccfail("unable to allocate space for new namespace");

        fncnsp->name = NULL;
        fncnsp->len = 0;

        fncnsp->type = CPL_FUNC;

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

        void * arbp = sbrk(0);

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

        brk(arbp);

        return;
    }

    if ( curtype->stype ) // TODO struct initialization
    {
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

        // TODO output floats
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

// (extern|static) (const|register) (signed|unsigned) <void|char|int|...|struct|union|...|etc>
void define( struct mccnsp * curnsp )
{
    #define LNK_EXTERN 1
    #define LNK_STATIC 2
    unsigned int8_t ctype = 0, cnsp = 0, lnk = 0;

    unsigned int8_t nsptype = curnsp->type & CPL_NSPACE_MASK;

    if ( nsptype == CPL_BLOCK )
    {
        // Determine linkage
        switch ( tk )
        {
            case Extern: lnk = LNK_EXTERN; ntok(); break;
            case Static: lnk = LNK_STATIC; ntok(); break;
        }

        // Check storage type
        switch ( tk ) // Check for storage type
        {
            case Const:    ctype |= CPL_CNST; ntok(); break;
            case Register: ctype |= CPL_ZERO; ntok(); break;
            case Auto:
            default: ctype |= curnsp == &glbnsp ? CPL_BSS : CPL_STAK;
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
        if ( tk == Named || tk == '{' )
        {
            // TODO struct overriding and anonymous child struct offset
            if ( tk == Named )
            {
                decnsp = findNamespace( curnsp, tkStr, tkVal );
            }

            if ( !decnsp || tk == '{' ) // Generate new struct
            {
                decnsp = sbrk( sizeof(struct mccnsp) );
                if ( decnsp == SBRKFAIL ) mccfail("unable to allocate space for new struct");

                if ( tk == Named )
                {
                    // Copy name
                    decnsp->name = sbrk( tkVal );
                    if ( decnsp->name == SBRKFAIL ) mccfail("unable to allocate space for new struct name");

                    int16_t i;
                    for ( i = 0; i < tkVal; i++ ) decnsp->name[i] = tkStr[i];
                    decnsp->len = tkVal;
                }
                else // Anonymous struct
                {
                    decnsp->name = NULL;
                    decnsp->len = 0;
                }

                // Record namespace type
                decnsp->type = cnsp;

                // Init tables
                decnsp->size = 0;
                decnsp->symtbl = NULL;
                decnsp->nsptbl = NULL;

                decnsp->symtail = &decnsp->symtbl;
                decnsp->nsptail = &decnsp->nsptbl;

                // Add to parent namespace
                decnsp->parent = curnsp;
                decnsp->next = NULL;

                *curnsp->nsptail = decnsp;
                curnsp->nsptail = &decnsp->next;
            }
            // Type check existing struct
            else if ( (decnsp->type & CPL_NSPACE_MASK) != (cnsp & CPL_NSPACE_MASK) ) mccfail("struct type missmatch");
        }
        else mccfail("expected struct name or anonymous struct definition");

        // Struct definition
        if ( tk == '{' )
        {
            // Don't redfine things
            if ( decnsp->type & CPL_DEFN ) mccfail("struct redefinition");
            decnsp->type |= CPL_DEFN;

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

        // TODO compare to existing symbol
//        compareSymbol( cursym,

        // Add symbol to current namespace (assuming it doesn't already exist)
        *curnsp->symtail = cursym;
        curnsp->symtail = &cursym->next;

        // Get innermost subtype
        struct mccsubtype * sbt;
        for ( sbt = cursym->type.sub; sbt->sub; sbt = sbt->sub );

        if ( sbt->ftype )
        {
            if ( curnsp != &glbnsp ) mccfail("function declared outside file scope");

            // Function with codeblock
            if ( tk == '{' )
            {
                // Mark function as defined
                if ( sbt->ftype->type & CPL_DEFN ) mccfail("function already declared");
                sbt->ftype->type |= CPL_DEFN;

                cursym->type.ptype &= ~CPL_STORE_MASK;
                cursym->type.ptype |= CPL_TEXT;

                write( segs[SEG_TEXT], cursym->name, cursym->len );
                write( segs[SEG_TEXT], ":\n", 2 );

                statement(sbt->ftype);
                break;
            }
        }
        else if ( (cursym->type.ptype & CPL_STORE_MASK) == CPL_STAK ) // This variable was declared on the stack
        {
            // Compute address
            unsigned int16_t tsz = typeSize(&cursym->type);
            if ( curnsp->size & 1 && tsz > 1 ) curnsp->size++; // Align to word if needed
            cursym->addr = curnsp->size;
            curnsp->size += tsz;

            // TODO stack declarations
        }
        else  // Not a function
        {
            // No longer initialized to zero so convert BSS to DATA
            if ( tk == Ass && (cursym->type.ptype & CPL_STORE_MASK) == CPL_BSS )
            {
                cursym->type.ptype &= ~CPL_STORE_MASK;
                cursym->type.ptype |= CPL_DATA;
            }

            int16_t segfd = segs[(cursym->type.ptype & CPL_STORE_MASK) >> 4];

            write( segfd, cursym->name, cursym->len );
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
        }

        // Only declare one variable per definition
        if ( nsptype == CPL_CAST
          || nsptype == CPL_FUNC
          || nsptype == CPL_VFUNC ) break;
        else if ( tk == Comma ) ntok();
        else if ( tk != ';' ) mccfail("expected comma or semi-colon");
    }

    if ( tk == ';' ) ntok();
}
