void declare( struct mccsym * cursym, struct mccsubtype ** partype )
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
        curtype->sub = sbrk( sizeof(struct mccsubtype) );
        if ( curtype->sub == SBRKFAIL ) mccfail("unable to allocate space for subtype");

        declare( cursym, &curtype->sub );

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

    // Get array declarations
    if ( tk == '[' )
    {
        // Pointer sizes to end of current program break
        curtype->sizes = sbrk(0);

        while ( tk == '[' )
        {
            unsigned int16_t * cas = sbrk( sizeof(unsigned int16_t) );
            if ( cas == SBRKFAIL ) mccfail("unable to allocate space for array size");

            // TODO resolve constant expression and store value to 'cas'

            curtype->arrays++;

            if ( tk != ']' ) mccfail("expected closing bracket");
        }
    }
    else if ( tk == '(' ) // Get function declarations
    {
        struct mccnsp * curnsp = curtype->ftype = sbrk( sizeof(struct mccnsp) );

        curnsp->name = NULL;
        curnsp->len = 0;

        curnsp->type = CPL_FUNC;

        curnsp->size = 0;

        curnsp->symtbl = NULL;
        curnsp->nsptbl = NULL;

        curnsp->symtail = &curnsp->symtbl;
        curnsp->nsptail = &curnsp->nsptbl;

        curnsp->parent = &glbnsp;
        curnsp->next = NULL;

        ntok();
        while ( tk != ')' )
        {
            // This is a variadic function
            if ( tk == Variadic )
            {
                curnsp->type = CPL_VFUNC;

                ntok();
                if ( tk != ')' ) mccfail("expected closing parenthasis 1");
                break;
            }

            define( curnsp );

            if ( tk == ',' ) ntok();
            else if ( tk != ')' ) mccfail("expected closing parenthasis 2");
        }

        ntok();
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

// (extern|static) (const|register) (signed|unsigned) <void|char|int|...etc>
void define( struct mccnsp * curnsp )
{
    #define LNK_XTRN 1
    #define LNK_STAT 2
    unsigned int8_t ctype = 0, cnsp = 0, lnk = 0;

    if ( (curnsp->type & CPL_NSPACE_MASK) == CPL_BLOCK )
    {
        // Determine linkage
        switch ( tk )
        {
            case Extern: lnk = LNK_XTRN; ntok(); break;
            case Static: lnk = LNK_STAT; ntok(); break;
        }

        // Check storage type
        switch ( tk ) // Check for storage type
        {
            case Const:    ctype |= CPL_TEXT; ntok(); break;
            case Register: ctype |= CPL_ZERO; ntok(); break;
            case Auto:
            default: ctype |= curnsp == &glbnsp ? CPL_DATA : CPL_STAK;
        }
    }

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

    struct mccnsp * newnsp = NULL;

    // Get struct name or anonymous definition
    if ( cnsp & CPL_NSPACE_MASK )
    {
        if ( tk == Named || tk == '{' )
        {
            if ( tk == Named )
            {
                newnsp = getNamespace( curnsp, tkStr, tkVal );
            }

            if ( !newnsp ) // Generate new struct
            {
                newnsp = sbrk( sizeof(struct mccnsp) );
                if ( newnsp == SBRKFAIL ) mccfail("unable to allocate space for new struct");

                if ( tk == Named )
                {
                    // Copy name
                    newnsp->name = sbrk( tkVal );
                    if ( newnsp->name == SBRKFAIL ) mccfail("unable to allocate space for new struct name");

                    int16_t i;
                    for ( i = 0; i < tkVal; i++ ) newnsp->name[i] = tkStr[i];
                    newnsp->len = tkVal;

                    ntok();
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
            }
            // Type check existing struct
            else if ( (newnsp->type & CPL_NSPACE_MASK) != (cnsp & CPL_NSPACE_MASK) ) mccfail("struct type missmatch");
        }
        else mccfail("expected struct name or anonymous struct definition");

        // Struct definition
        if ( tk == '{' )
        {
            // Don't redfine things
            if ( newnsp->type & CPL_DEFN ) mccfail("struct redefinition");
            newnsp->type |= CPL_DEFN;

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
                    define(newnsp);
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
        Count number of inderections
        */

        struct mccsym * cursym = sbrk( sizeof(struct mccsym) );
        if ( cursym == SBRKFAIL ) mccfail( "unable to allocate space for new symbol" );

        cursym->name = NULL;
        cursym->len = 0;

        cursym->type.ptype = ctype; // Record primative datatype
        cursym->type.stype = newnsp; // Record if this is a struct or not

        cursym->addr = 0;

        cursym->next = NULL;

        cursym->type.sub = sbrk( sizeof(struct mccsubtype) );
        if ( cursym->type.sub == SBRKFAIL ) mccfail( "unable to allocate space for new symbol subtype" );

        // Process type information
        declare( cursym, &cursym->type.sub );

//	TODO compare to existing symbol
//        compareSymbol( cursym,

        // Add symbol to current namespace (assuming it doesn't already exist)
        *curnsp->symtail = cursym;
        curnsp->symtail = &cursym->next;

        // Get innermost subtype
        struct mccsubtype * sbt;
        for ( sbt = cursym->type.sub; sbt->sub; sbt = sbt->sub );

        // Function call
        if ( sbt->ftype && tk == '{' )
        {
            // Mark function as defined
            if ( sbt->ftype->type & CPL_DEFN ) mccfail("function already declared");
            sbt->ftype->type |= CPL_DEFN;

            // Create namespace to hold all bottom level function declarations
            struct mccnsp * bbnsp = sbrk(sizeof(struct mccnsp));
            if ( bbnsp == SBRKFAIL ) mccfail("unable to allocate room for function base block");

            bbnsp->name = NULL;
            bbnsp->len = 0;

            bbnsp->type = CPL_BLOCK;
            bbnsp->size = 0;

            bbnsp->symtbl = NULL; bbnsp->symtail = &bbnsp->symtbl;
            bbnsp->nsptbl = NULL; bbnsp->nsptail = &bbnsp->nsptbl;

            bbnsp->parent = sbt->ftype;
            bbnsp->next = NULL;

            // Add to parent namespace
            *sbt->ftype->nsptail = bbnsp;
            sbt->ftype->nsptail = &bbnsp->next;

            ntok();
            while ( tk != '}' )
            {
                statement(bbnsp);
            }

            ntok();
            break;
        }

        // Only declare one variable per definition
        if ( (curnsp->type & CPL_NSPACE_MASK) == CPL_FUNC
          || (curnsp->type & CPL_NSPACE_MASK) == CPL_VFUNC ) break;
        else if ( tk == ',' ) ntok();
        else if ( tk != ';' ) mccfail("expected comma or semi-colon");
    }

    if ( tk == ';' ) ntok();
}
