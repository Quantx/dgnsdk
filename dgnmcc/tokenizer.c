// File info
int16_t sfd;
unsigned int16_t ln; // Current line number
int8_t * fp; // Name of the file

// Line into
int8_t lp[MAX_LINE]; // Stores current line
int8_t * p = lp, * pp;

// Token information
unsigned int8_t tk;
int16_t tkVal;
int32_t tkLong;
int8_t tkStr[256];

// Reserved words (MUST MATCH ORDER IN TOKEN ENUM)
int8_t * res_words[] = {
    "void",
    "int",
    "short",
    "char",
    "float",
    "long",
    "enum",
    "struct",
    "union",
    "auto",
    "static",
    "register",
    "const",
    "extern",
    "signed",
    "unsigned",
    "if",
    "else",
    "case",
    "default",
    "break",
    "continue",
    "return",
    "for",
    "while",
    "do",
    "goto",
    "sizeof"
};

int16_t readline()
{
    int16_t i = 0;

    // Read data in and scan for newline
    while ( i < MAX_LINE - 1 && read( sfd, lp + i, 1 ) && lp[i++] != '\n' );
    if ( i == MAX_LINE - 1 ) mccfail("readline overflow");

    // Null terminate a line if needed
    lp[i] = 0;

    // Increment line count
    ln++;

    p = lp;

    return i;
}

#ifdef DEBUG_TOKEN
void debug_ntok()
#else
void ntok()
#endif
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
                while ( *p != '*' || p[1] != '/' )
                    if ( !*p++ && !readline() )
                        mccfail("expected block comment terminator, got end of file");

                p += 2;
                continue;
            }
            // Division
            else if ( *p == '=' ) { p++; tk = DivAss; }
            else tk = Div;
            return;
        }
        // Named symbol
        else if ( tk >= 'A' && tk <= 'Z' || tk >= 'a' && tk <= 'z' || tk == '_' )
        {
            int16_t i;

            // Get entire symbol
            while ( *p >= 'a' && *p <= 'z'
                 || *p >= 'A' && *p <= 'Z'
                 || *p >= '0' && *p <= '9'
                 || *p == '_' ) p++;

            if ( p - pp > 255 ) mccfail("named token exceedes max character length");
            tkVal = p - pp;

            // Check for reserved word
            for ( tk = Void; tk <= SizeofRes; tk++ )
            {
                for ( i = 0; res_words[tk - Void][i] == pp[i]; i++ );

                if ( !res_words[tk - Void][i] ) // Match
                {
                     if ( tk == Short  ) tk = Int; // Shorts are always Ints
                     if ( tk == SizeofRes ) tk = Sizeof; // Sizeof is located in a weird spot
                     return;
                }
            }

            // User defined Named
            tk = Named;
            for ( i = 0; i < tkVal; i++ ) tkStr[i] = pp[i];
            tkStr[i] = 0;

            return;
        }
        // Numerical constant
        else if ( tk >= '0' && tk <= '9' )
        {
            tkLong = 0;

            if ( tk != '0' ) // Decimal
            {
                tkLong = tk - '0';
                while ( *p >= '0' && *p <= '9' ) tkLong = (tkLong << 3) + tkLong + tkLong + *p++ - '0';
            }
            else if ( *p == 'x' || *p == 'X' ) // Hexadecimal
            {
                p++;
                while ( *p >= '0' && *p <= '9'
                     || *p >= 'a' && *p <= 'f'
                     || *p >= 'A' && *p <= 'F' )
                    tkLong = (tkLong << 4) + (*p & 0xF) + (*p++ >= 'A' ? 9 : 0);
            }
            else if ( *p == 'b' || *p == 'B' ) // Binary
            {
                p++;
                while ( *p == '0' || *p == '1' ) tkLong = (tkLong << 1) + *p++ - '0';
            }
            else // Octal
            {
                while ( *p >= '0' && *p <= '7' ) tkLong = (tkLong << 3) + *p++ - '0';
            }

            tkVal = (int16_t)tkLong;

            if ( *p == 'l' || *p == 'L' || tkLong >= 0xFFFF ) { p++; tk = LongNumber; }
            else if ( *p == 'c' || *p == 'C' || (unsigned int16_t)tkVal <= 0xFF ) { p++; tk = SmolNumber; }
            else tk = Number;

            return;
        }
        else if ( tk == '\'' || tk == '"' ) // Character or String constant
        {
            // Record initial value
            if ( tk == '"' ) tkVal = cnst.pos;
            else tkVal = *p;

            while ( *p || readline() )
            {
                int8_t out = *p;

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
                    // TODO write string to const segment
                }
                else if ( *p != '\'' ) mccfail("Character constant too long");
                // Record character constant
                else
                {
                    tk = SmolNumber;
                    tkVal = out;
                }
            }

            // End of input feed
            mccfail("failed to find closing quotation on string");
        }
        else if ( tk == '.' )
        {
            if ( *p == '.' && p[1] == '.' ) { p += 2; tk = Variadic; }
            else tk = Dot;
            return;
        }
        else if ( tk == '=' )
        {
            if ( *p == '=' ) { p++; tk = Eq; }
            else tk = Ass;
            return;
        }
        else if ( tk == '+' )
        {
            if ( *p == '+' ) { p++; tk = PostInc; }
            else if ( *p == '=' ) { p++; tk = AddAss; }
            else tk = Add;
            return;
        }
        else if ( tk == '-' )
        {
            if ( *p == '-' ) { p++; tk = PostDec; }
            else if ( *p == '=' ) { p++; tk = SubAss; }
            else if ( *p == '>' ) { p++; tk = Arrow; }
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
        else if ( tk == '?' || tk == ':'
               || tk == ',' || tk == ';'
               || tk == '[' || tk == ']'
               || tk == '{' || tk == '}'
               || tk == '(' || tk == ')' ) return;
    }

    tk = 0; // End of input feed
}

#ifdef DEBUG_TOKEN
void ntok()
{
    debug_ntok();

    write( 1, "TOKEN: ", 7 );
    octwrite( 1, tk );

    if ( tk )
    {
        write( 1, ": ", 2 );
        write( 1, pp, p - pp );
    }

    write( 1, "\r\n", 2 );
}
#endif
