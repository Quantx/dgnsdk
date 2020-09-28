void compile()
{
    tk = 1;
    while ( tk )
    {
        define(&glbnsp);
    }
}

void declare( struct mccnsp * curnsp, struct mccnsp * nsptype, char ctype, char lnk )
{
    // Count number of inderections
    unsigned char aster = 0;
    while ( tk == Mul ) { aster++; ntok(); }

    struct mccfunc * functype = NULL;

    if ( tk == '(' ) // Function pointer declaration
    {
        functype = sbrk( sizeof(struct mccfunc) );
        if ( functype == SBRKFAIL ) mccfail( "unable to allocate space for func ptr type" );

        functype->size = 0;
        functype->inder = 0;
        functype->argc = 0;

        functype->argnsp = NULL;

        ntok();
        while ( tk == Mul ) { functype->inder++; ntok(); }
    }

    if ( tk == Nspace ) // mccfail( "tried to use a struct type as a symbol" );
    if ( tk == Symbol ) // mccfail( "symbol already declared" );
    if ( tk != Named  ) mccfail( "expected name in declaration" );

    struct mccsym * cursym = sbrk( sizeof(struct mccsym) );
    if ( cursym == SBRKFAIL ) mccfail( "unable to allocate space for new symbol" );

    cursym->name = tkPtr;
    cursym->len = tkVal;

    cursym->type.type = ctype; // Record type information
    cursym->type.inder = aster; // Record number of inderections

    cursym->type.ftype = functype; // Record function information
    cursym->type.stype = curnsp; // Record if this is a struct or not

    ntok();

    if ( functype || tk == '(' ) // Function declaration
    {
        if ( functype )
        {
            if ( tk == Brak )
            {
                ntok();

                if ( tk == Number )
                {
                    if ( tkVal <= 0 ) mccfail( "func ptr array size must be at least 1" );

                    functype->size = tkVal;

                    ntok();
                }
                else if ( tk == ']' ) functype->size = 1;

                if ( tk != ']' ) mccfail( "expected closing bracket on func ptr array" );

                ntok();
            }

            if ( tk != ')' ) mccfail( "expected closing parenthasis" );

            ntok();

            if ( tk != '(' ) mccfail( "expected func args following func ptr" );
        }
        else // Function definition
        {
            functype = sbrk( sizeof(struct mccfunc) );
            if ( functype == SBRKFAIL ) mccfail( "unable to allocate space for func definition" );

            functype->size = 0;
            functype->inder = 0;
            functype->argc = 0;

            functype->argnsp = NULL;

            cursym->type.ftype = functype;
        }

        if ( curnsp != &glbnsp && !functype->size && !functype->inder ) mccfail("cannot declare function outside of file scope");

        // Create argument namespace
        functype->argnsp = sbrk( sizeof(struct mccnsp) );
        if ( functype->argnsp == SBRKFAIL ) mccfail( "unable to allocate space for func arg namespace" );

        functype->argnsp->name = NULL;
        functype->argnsp->len = 0;

        functype->argnsp->type = CPL_FUNC;
        functype->argnsp->size = 0;

        functype->argnsp->symtbl = NULL;
        functype->argnsp->nsptbl = NULL;

        functype->argnsp->symtail = &functype->argnsp->symtbl;
        functype->argnsp->nsptail = &functype->argnsp->nsptbl;

        functype->argnsp->parent = curnsp;
        functype->argnsp->next = NULL;

        ntok();

        while ( tk != ')' )
        {
            struct mccnsp * ansp = NULL;

            if ( tk == Struct || tk == Enum || tk == Union )
            {
                ntok();

                if ( tk != Nspace ) mccfail( "expected namespace" );

                ansp = tkPtr;

                ntok();
            }

            unsigned char argaster = 0;
            while ( tk == Mul ) { argaster++; ntok(); }

            // TODO read in arguments
        }
    }
    else if ( tk == '=' ) // Assignment
    {
        ntok();

        if ( tk == '{' )
        {
            while ( tk != '}' )
            {

            }
        }
        else if ( tk != Number ) mccfail("expected constant value");

        ntok();
    }
    else if ( tk == Brak ) // Array declaration
    {
        ntok();
        if ( tk == Number )
        {
            if ( tkVal <= 0 ) mccfail( "array size must be at least 1" );

            cursym->type.size = tkVal;

            ntok();
        }
        else if ( tk == ']' ) cursym->type.size = 1;

        if ( tk != ']' ) mccfail( "expected constant value" );

        ntok();
    }

    if ( tk == ',' ) ntok();
    else if ( tk != ';' ) mccfail( "expected comma or end of declaration" );
}

// (extern|static) (const|register) (signed|unsigned) <void|char|int|...etc>
void define( struct mccnsp * curnsp )
{
    #define LNK_XTRN 1
    #define LNK_STAT 2
    unsigned char ctype = 0, cnsp = 0, lnk = 0;

    tkNsp = curnsp;

    ntok();

    if ( curnsp->type == CPL_FILE || curnsp->type == CPL_BLOC )
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
            default: ctype |= CPL_DATA;
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
        case Union:  cnsp  |= CPL_UNIN; break;

        default: mccfail("missing type in declaration");
    }

    ntok();

    struct mccnsp * newnsp = NULL;

    // Get struct name or anonymous definition
    if ( cnsp & CPL_NSPACE_MASK )
    {
        int scope = -1;

        if ( tk == Named || tk == Symbol ) // New struct
        {
            if ( tk == Symbol )
            {
                struct mccsym * tmpsym = tkPtr;

                tk = Named;
                tkPtr = tmpsym->name;
                tkVal = tmpsym->len;
            }

            newnsp = sbrk( sizeof(struct mccnsp) );
            if ( newnsp == SBRKFAIL ) mccfail("unable to allocate space for new struct");

            // Name of the namespace
            newnsp->name = tkPtr;
            newnsp->len = tkVal;

            ntok();
        }
        else if ( tk == Nspace ) // Existing struct
        {
            newnsp = tkPtr;
            scope = tkVal;
            ntok();
        }
        else if ( tk != '{' ) mccfail("expected struct name or anonymous struct definition");

        // Struct definition
        if ( tk == '{' )
        {
            if ( scope > 0 ) // Create a new struct in this scope
            {
                struct mccnsp * tmpnsp = newnsp;

                newnsp = sbrk( sizeof(struct mccnsp) );
                if ( newnsp == SBRKFAIL ) mccfail("unable to allocate space for new struct");

                // Copy name
                for ( int i = 0; i < tmpnsp->len; i++ ) newnsp->name[i] = tmpnsp->name[i];
                newnsp->len = tmpnsp->len;
            }

            if ( !scope ) mccfail("struct already defined at this scope");
            else // Setup rest of struct
            {
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

            while ( tk != '}' )
            {
                define(newnsp);
            }

            tkNsp = curnsp;
            ntok(); // Discard last }
        }
    }

    // Process declarations
    while ( tk != ';' )
    {
        declare( curnsp, newnsp, ctype, lnk );
    }
}
