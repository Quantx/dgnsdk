#include "mcc0.h"

// Line into
unsigned int32_t ctn;
unsigned int32_t ttn;
unsigned int16_t ln; // Current line number
int8_t lp[MAX_LINE]; // Stores current line
int8_t * p, * pp;

// Token information
unsigned int8_t tk;
int16_t tkVal;
int32_t tkLong;
int8_t tkStr[MAX_STR];

// Reserved words (MUST MATCH ORDER IN TOKEN ENUM)
int8_t * res_words[30] = {
    "void", "int", "short", "char", "float", "long", "double",
    "enum", "struct", "union",
    "signed", "unsigned",
    "extern",
    "auto", "static", "register", "const",
    "if", "else", "switch", "case", "default",
    "break", "continue", "return",
    "for", "while", "do",
    "goto",
    "sizeof"
};

void mccfail( int8_t * msg )
{
    decwrite( 2, ln );
    write( 2, ":", 1 );
    decwrite( 2, p - lp );
    write( 2, ":", 1 );

    die( msg );
}

int16_t readline()
{
    int16_t i = 0;

    // Read data in and scan for newline
    while ( i < MAX_LINE - 1 && read( 0, lp + i, 1 ) && lp[i++] != '\n' );
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

        if ( tk >= 'A' && tk <= 'Z' || tk >= 'a' && tk <= 'z' || tk == '_' )
        {
            int16_t i;

            // Get entire symbol
            while ( *p >= 'a' && *p <= 'z'
                 || *p >= 'A' && *p <= 'Z'
                 || *p >= '0' && *p <= '9'
                 || *p == '_' ) p++;

            // This MUST be 255 because that's the maximum var name size
            if ( p - pp > 255 ) mccfail("named token exceedes max character length");
            tkVal = p - pp;

            // Check for reserved word
            for ( tk = Void; tk <= SizeofRes; tk++ )
            {
                for ( i = 0; res_words[tk - Void][i] == pp[i]; i++ );

                if ( !res_words[tk - Void][i] && tkVal == i ) // Match
                {
                     if ( tk == Short ) tk = Int; // Shorts are always Ints
                     if ( tk == SizeofRes ) tk = Sizeof; // Sizeof is located in a weird spot
                     return;
                }
            }

            // User defined Named
            tk = Named;
            for ( i = 0; i < tkVal; i++ ) tkStr[i] = pp[i];

            return;
        }
        // Numerical constant
        else if ( tk >= '0' && tk <= '9' )
        {
            // TODO-FPU process floating point constants

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

            tkVal = (int16_t CAST_NAME)tkLong;

            if ( *p == 'l' || *p == 'L' ) { p++; tk = LongNumber; }
            else if ( (unsigned int32_t CAST_NAME)tkLong > 0xFFFF ) tk = LongNumber;

            else if ( *p == 'c' || *p == 'C' ) { p++; tk = SmolNumber; }
            else if ( (unsigned int16_t CAST_NAME)tkVal <= 0xFF ) tk = SmolNumber;

            else tk = Number;

            return;
        }
        else switch ( tk )
        {
            // End of line
            case '\n': *p = 0; continue;
            // Single line comment
            case '/':
                // Comment
                if ( *p == '/' ) { *p = 0; continue; }
                if ( *p == '*' )
                {
                    // Multi-line comment, hunt end
                    while ( *p != '*' || p[1] != '/' )
                        if ( !*p++ && !readline() )
                            mccfail("expected block comment terminator, got end of file");

                    p += 2;
                    continue;
                }
                // Division
                if ( *p == '=' ) { p++; tk = DivAss; return; }
                tk = Div;
                return;
            // Character constant
            case '\'':;
                int8_t out = *p;

                // Escape character
                if ( *p++ == '\\' )
                {
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

                if ( *p++ != '\'' ) mccfail("missing end quote on character constant");

                // Record character constant
                tk = SmolNumber;
                tkVal = out;
                return;
            // String constant
            case '"':
                for ( tkVal = 0; *p || readline(); tkVal++ ) // Continuous read
                {
                    tkStr[tkVal] = *p;

//	            write( 2, p, 1 );

                    if ( *p == '"' && tk != '\\' )
                    {
                        p++;
                        tk = String;
                        return;
                    }

                    tk = *p++;
                }

                mccfail("missing closing quote on string constant");
            case '.': if ( *p == '.' && p[1] == '.' ) { p += 2; tk = Variadic; return; } tk = Dot; return;
            case '=': if ( *p == '=' ) { p++; tk = Eq; return; } tk = Ass; return;
            case '+':
                if ( *p == '+' ) { p++; tk = PostInc; return; }
                if ( *p == '=' ) { p++; tk = AddAss; return; }
                tk = Add;
                return;
            case '-':
                if ( *p == '-' ) { p++; tk = PostDec; return; }
                if ( *p == '=' ) { p++; tk = SubAss; return; }
                if ( *p == '>' ) { p++; tk = Arrow; return; }
                tk = Sub;
                return;
            case '!': if ( *p == '=' ) { p++; tk = Neq; return; } tk = LogNot; return;
            case '<':
                if ( *p == '<' )
                {
                    if ( *++p == '=' ) { p++; tk = ShlAss; return; }
                    tk = Shl;
                    return;
                }
                if ( *p == '=' ) { p++; tk = LessEq; return; }
                tk = Less;
                return;
            case '>':
                if ( *p == '>' )
                {
                    if ( *++p == '=' ) { p++; tk = ShrAss; return; }
                    tk = Shr;
                    return;
                }
                if ( *p == '=' ) { p++; tk = GreatEq; return; }
                tk = Great;
                return;
            case '|':
                if ( *p == '|' ) { p++; tk = LogOr; return; }
                if ( *p == '=' ) { p++; tk = OrAss; return; }
                tk = Or;
                return;
            case '&':
                if ( *p == '&' ) { p++; tk = LogAnd; return; }
                if ( *p == '=' ) { p++; tk = AndAss; return; }
                tk = And;
                return;
            case '^': if ( *p == '=' ) { p++; tk = XorAss; return; } tk = Xor; return;
            case '%': if ( *p == '=' ) { p++; tk = ModAss; return; } tk = Mod; return;
            case '*': if ( *p == '=' ) { p++; tk = MulAss; return; } tk = Mul; return;
            case '~': tk = Not; return;
            case ',': tk = Comma; return;
            case ';': tk = SemiColon; return;
            case '?': tk = Q_Mark; return;
            case ':': tk = Colon; return;
            case '[': tk = Square_L; return;
            case ']': tk = Square_R; return;
            case '{': tk = Curly_L; return;
            case '}': tk = Curly_R; return;
            case '(': tk = Paren_L; return;
            case ')': tk = Paren_R; return;
        }
    }

    tk = EndOfFile; // End of input feed
}

int main( int argc, char ** argv )
{
    p = lp;

    if ( argc == 3 )
    {
        char * ttns = argv[1];
        while ( *ttns >= '0' && *ttns <= '7' ) ttn = (ttn << 3) + *ttns++ - '0';
        while ( ctn < ttn )
        {
            ntok();
            if ( !tk )
            {
                write( 2, "mcc0: unexpected eof\n", 21 );
                return 1;
            }

            ctn++;
        }

        unsigned int16_t hof = (unsigned int16_t)(p - lp);

        decwrite( 2, ln );
        write( 2, ":", 1 );
        decwrite( 2, hof );
        write( 2, ": ", 2 );

        int8_t * msg = argv[2];
        int16_t i;
        for ( i = 0; msg[i]; i++ );
        write( 2, msg, i );
        write( 2, "\r\n", 2 );

        for ( i = 0; lp[i]; i++ );
        write( 2, lp, i );

        for ( i = 0; i < hof; i++ )
        {
            write( 2, lp[i] == '\t' ? "\t" : " ", 1 );
        }
        write( 2, "^\r\n", 3 );

        return 0;
    }

    while ( ntok(), tk != EndOfFile )
    {

        write( 1, &tk, 1 ); // Output token

        // Output additional information
        switch ( tk )
        {
            case Named:
            case String:
                write( 1, &tkVal, 2 );
                write( 1, tkStr, tkVal );
                break;
            case Number:
            case SmolNumber:
                write( 1, &tkVal, 2 );
                break;
            case LongNumber:
                write( 1, &tkLong, 4 );
                break;
        }
    }

    return 0;
}
