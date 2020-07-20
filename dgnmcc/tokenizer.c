// File info
int fd;
unsigned int ln; // Current line number
char * fp; // Name of the file

// Line into
char * p, * pp;
char lp[MAX_LINE]; // Current line

int tkVal, cursf = -1;
unsigned char tk;

// Reserved words (MUST MATCH ORDER IN TOKEN ENUM)
char * res_words = "void int short char float long enum struct union auto static register const extern signed unsigned if else case default break continue return for while do sizeof";

int readline()
{
    int i = 0;

    // Read data in and scan for newline
    while ( i < MAX_LINE - 1 && read( fd, lp + i, 1 ) && lp[i++] != '\n' );
    if ( i == MAX_LINE - 1 ) mccfail("readline overflow");

    // Null terminate a line if needed
    lp[i] = 0;

    // Increment line count
    ln++;

    p = lp;

    return i;
}

void ntok()
{
    // Proccess line and token
    while ( *p || readline() )
    {
        tk = *p;
        pp = p++;

        // End of line
        if ( tk == '\n' ) { *p = 0; continue; }
        // Single line comment
        else if ( tk == '/' )
        {
            // Comment
            if ( *p == '/' ) { *p = 0; continue; }
            else if ( *p == '*' )
            {
                // Multi-line comment, hunt end
                while ( *p != '*' && p[1] != '/' )
                    if ( !*p++ && !readline() )
                        mccfail("expected block comment terminator, got end of file");

                p += 2;
                continue;
            }
            // Division
            else if ( *p == '=' ) tk = DivAss;
            else tk = Div;
            return;
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

            int toklen = p - pp;

            if ( toklen > MAX_TOKN ) mccfail("named token exceedes max character length");

            // Check for reserved word
            tk = Void;
            char * rpos = res_words;
            while ( *rpos )
            {
                int i = 0;
                while ( i < toklen && rpos[i] && rpos[i] != ' ' && rpos[i] == pp[i] ) i++;

                if ( rpos[i] == ' ' ) // Match
                {
                     if ( tk == Short ) tk = Int; // Shorts are always Ints
                     return;
                }

                while ( *rpos && *rpos++ != ' ' );

                tk++;
            }

            int i;

            // Check for named symbol
            tk = Named;
            tkVal = 0;
            while ( tkVal < sympos )
            {
                // Find matching symbol
                i = 0;
                while ( i < toklen && i < symtbl[tkVal].len && symtbl[tkVal].len == pp[i] ) i++;
                if ( i == toklen && toklen == symtbl[tkVal].len ) return;
            }

            // New symbol
            if ( sbrk( sizeof(struct mccsym) ) == SBRKFAIL ) mccfail("cannot allocate room for new symbol");

            // Copy name
            i = 0;
            while ( i < toklen ) { symtbl[tkVal].name[i] = pp[i]; i++; }

            // Record length
            symtbl[tkVal].len = toklen;

            // Ensure type is zero
            symtbl[tkVal].type = 0;

            return;
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
        else if ( tk == '\'' || tk == '"' ) // Character or String constant
        {
            // Record initial value
            if ( tk == '"' ) tkVal = cnst.data.pos;
            else tkVal = *p;

            while ( *p || readline() )
            {
                char out = *p;

                // Closing quotation
                if ( *p == tk ) { tk = Number; return; }

                // Escape character
                if ( *p == '\\' )
                {
                    p++;

                    if      ( *p == 'a'  ) { out = '\a'; p++; }
                    else if ( *p == 'b'  ) { out = '\b'; p++; }
                    else if ( *p == 'e'  ) { out = '\e'; p++; }
                    else if ( *p == 'f'  ) { out = '\f'; p++; }
                    else if ( *p == 'n'  ) { out = '\n'; p++; }
                    else if ( *p == 'r'  ) { out = '\r'; p++; }
                    else if ( *p == 't'  ) { out = '\t'; p++; }
                    else if ( *p == 'v'  ) { out = '\v'; p++; }
                    else if ( *p == '\\' ) { out = '\\'; p++; }
                    else if ( *p == '\'' ) { out = '\''; p++; }
                    else if ( *p == '"'  ) { out = '"';  p++; }
                    else if ( *p == 'x' )
                    {
                        if ( *++p >= '0' && *p <= '9'
                            || *p >= 'a' && *p <= 'f'
                            || *p >= 'A' && *p <= 'F' )
                            // Convert first digit
                            out = (*p & 0xF) + (*p++ >= 'A' ? 9 : 0);
                        else mccfail("expected two hex digits following a \\x escape");

                        // Convert possible second digit
                        if ( *p >= '0' && *p <= '9'
                          || *p >= 'a' && *p <= 'f'
                          || *p >= 'A' && *p <= 'F' )
                            out = (out << 4) + (*p & 0xF) + (*p++ >= 'A' ? 9 : 0);
                    }
                    else if ( *p >= '0' && *p <= '7' )
                    {
                        out = *p++ - '0'; // First digit

                        if ( *p >= '0' && *p <= '7' ) // Second digit
                            out = (out << 3) + *p++ - '0';

                        if ( *p >= '0' && *p <= '7' ) // Last digit
                            out = (out << 3) + *p++ - '0';
                    }
                    else mccfail("unknown escape sequence");
                }
                else p++;

                // Write string to cnst segment
                if ( tk == '"' )
                {
                    // TODO
                }
                else if ( *p != '\'' ) mccfail("Character constant too long");
                // Record character constant
                else tkVal = out;
            }

            // End of input feed
            mccfail("failed to find closing quotation on string");
        }
        else if ( tk == '=' )
        {
            if ( *p == '=' ) { p++; tk = Eq; }
            else tk = Ass;
            return;
        }
        else if ( tk == '+' )
        {
            if ( *p == '+' ) { p++; tk = Inc; }
            else if ( *p == '=' ) { p++; tk = AddAss; }
            else tk = Add;
            return;
        }
        else if ( tk == '-' )
        {
            if ( *p == '-' ) { p++; tk = Dec; }
            else if ( *p == '=' ) { p++; tk = SubAss; }
            else tk = Sub;
            return;
        }
        else if ( tk == '!' )
        {
            if ( *p == '=' ) { p++; tk = Neq; }
            else tk = LogNot;
            return;
        }
        else if ( tk == '<' )
        {
            if ( *p == '<' )
            {
                if ( *++p == '=' ) { p++; tk = ShlAss; }
                else tk = Shl;
            }
            else if ( *p == '=' ) { p++; tk = LessEq; }
            else tk = Less;
            return;
        }
        else if ( tk == '>' )
        {
            if ( *p == '>' )
            {
                if ( *++p == '=' ) { p++; tk = ShrAss; }
                else tk = Shr;
            }
            else if ( *p == '=' ) { p++; tk = GreatEq; }
            else tk = Great;
            return;
        }
        else if ( tk == '|' )
        {
            if ( *p == '|' ) { p++; tk = LogOr; }
            else if ( *p == '=' ) { p++; tk = OrAss; }
            else tk = Or;
            return;
        }
        else if ( tk == '&' )
        {
            if ( *p == '&' ) { p++; tk = LogAnd; }
            else if ( *p == '=' ) { p++; tk = AndAss; }
            else tk = And;
            return;
        }
        else if ( tk == '^' )
        {
            if ( *p == '=' ) { p++; tk = XorAss; }
            else tk = Xor;
            return;
        }
        else if ( tk == '~' ) { tk = Not; return; }
        else if ( tk == '%' )
        {
            if ( *p == '=' ) { p++; tk = ModAss; }
            else tk = Mod;
            return;
        }
        else if ( tk == '*' )
        {
            if ( *p == '=' ) { p++; tk = MulAss; }
            else tk = Mul;
            return;
        }
        else if ( tk == '?' ) { tk = Tern; return; }
        else if ( tk == '[' ) { tk = Brak; return; }
        else if ( tk == ']' || ',' || tk == ';' || tk == ':' || tk == '{' || tk == '}' || tk == '(' || tk == ')' ) return;
    }

    tk = 0; // End of input feed
}
