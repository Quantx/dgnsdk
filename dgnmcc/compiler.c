void compile()
{
    tk = 1;
    while ( tk )
    {
	// Get specifiers and qualifiers
#define STORE_TYPE  1
#define STORE_CLASS 1 << 1
#define STORE_XTRN  1 << 2
#define STORE_SIGN  1 << 3
        char store = 0, storeFlags = 0;

        ntok();
        while ( tk >= Auto && tk <= Unsigned )
        {
            if ( tk <= Union ) // Storage type
            {
                if ( storeFlags & STORE_TYPE ) mccfail("storage type already specified");
                storeFlags |= STORE_TYPE;

                switch ( tk )
                {
                    case Void:   store |= CPL_VOID;
                    case Char:   store |= CPL_CHAR;
                    case Int:    store |= CPL_INT;
                    case Long:   store |= CPL_LONG;
                    case Float:  store |= CPL_FPV;
                    case Struct: store |= CPL_STRC;
                    case Enum:   store |= CPL_ENUM;
                    case Union:  store |= CPL_UNI;
                }
            }
            else if ( tk <= Const ) // Storage class
            {
                if ( storeFlags & STORE_CLASS ) mccfail("storage class already specified" );
                storeFlags |= STORE_CLASS;

                switch ( tk )
                {
                    case Const:    store |= CPL_TEXT;
                    case Register: store |= CPL_ZERO;
                    case Static:   store |= CPL_DATA;
                    case Auto:     store |= CPL_STAK;
                }
            }
            // Storage Qualifiers
            else if ( tk == Extern )
            {
                if ( storeFlags & STORE_XTRN ) mccfail("extern already specified");
                storeFlags |= STORE_XTRN;

                store |= CPL_XTRN;
            }
            else
            {

                if ( storeFlags & STORE_SIGN ) mccfail("extern already specified");
                storeFlags |= STORE_SIGN;

                if ( tk == Signed ) store |= CPL_SIGN;
            }

            ntok();
        }

        // Process prototypes
        unsigned char inder = 0;
        while ( tk == Mul ) { inder++; ntok(); }

        // Process declarations
        do
        {
            // Count number of inderections
            unsigned char inder = 0;
            while ( tk == Mul ) { inder++; ntok(); }

            if ( tk != Named ) mccfail( "expected name in declaration" );

            struct mccsym * cursym = tkSym;

            ntok();

            if ( tk == '=' ) // Assignment
            {
                ntok();

                
            }
            else if ( tk == Brak ) // Array declaration
            {
                inder++;

                ntok();
                if ( tk == Number )
                {
                    if ( tkVal <= 0 ) mccfail( "array size must be at least 1" );

                    tkSym->size = tkVal;

                    ntok();
                }

                if ( tk != ']' ) mccfail( "expected numerical constant" );

                ntok();
            }
            else if ( tk == '{' ) // Member declaration
            {
                
            }
        }
        while ( tk == ',' );
    }
}
