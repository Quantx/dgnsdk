char lp[MAX_LINE], * pp, * p;

int readline()
{

}

void ntok()
{
    do
    {
        while ( tk = *p )
        {
            pp = p++;

            // End of line
            if ( tk == '\n' ) break;
            // Single line comment
            else if ( tk == '/' ) if ( *p == '/' ) { p++; break; } else if ( *p == '*' )
            {
                // Multi-line comment, hunt end
                while ( *p != '*' && p[1] != '/' )
                    if ( !*p++ && !readline() )
                        mccfail("expected block comment terminator, got end of file");

                p += 2;
                break;
            }
            // Named symbol
            else if ( tk >= 'A' && tk <= 'Z' || tk >= 'a' && tk <= 'z' || tk == '_' )
            {
                // Get entire symbol
                while ( p - pp <= MAX_TOKN &&
                    ( *p >= 'a' && *p <= 'z'
                   || *p >= 'A' && *p <= 'Z'
                   || *p >= '0' && *p <= '9'
                   || *p == '_' ) ) p++;

                if ( p - pp > MAX_TOKN ) mccfail("named token exceedes max character length");

                
            }
            // Numerical constant
            else if ( tk >= '0' && tk <= '9' )
            {
                tkVal = 0;

                if ( tk != '0' ) // Decimal
                {
                    tkVal = tk - '0';
                    while ( *p >= '0' && *p <= '9' ) tkVal = tkVal * 10 + *p++ - '0';
                }
                else if ( *p == 'x' || *p == 'X' ) // Hexadecimal
                {
                    p++;
                    while ( *p >= '0' && *p <= '9'
                         || *p >= 'a' && *p <= 'f'
                         || *p >= 'A' && *p <= 'F' )
                        tkVal = (tkVal << 4) + (*p & 0xF) + (*p++ >= 'A' ? 9 : 0);
                }
                else if ( *p == 'b' || *p == 'B' ) // Binary
                {
                    p++;
                    while ( *p == '0' || *p == '1' ) tkVal = (tkVal << 1) + *p++ - '0';
                }
                else // Octal
                {
                    while ( *p >= '0' && *p <= '7' ) tkVal = (tkVal << 3) + *p++ - '0';
                }

                tk = Number;
                return;
            }
        }
    }
    while ( readline() )

    tk = 0; // End of file
}
