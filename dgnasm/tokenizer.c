char lp[MAX_LINE], * p, * pp, * fp = NULL, tk, ustr[MAX_STR + 1], * usp;
unsigned int curline;
int fd, tkVal;

int readline()
{
    // Current line still has data
    if ( tk != TOK_EOL ) return -1;

    int i = 0;

    // Read data in and scan for newline
    while ( i < MAX_LINE - 1 && read( fd, lp + i, 1 ) && lp[i++] != '\n' );

    // Buffer overflow
    if ( i == MAX_LINE - 1 ) asmfail("readline overflow");

    // Null terminate a line if needed
    lp[i] = 0;

    // Increment line count
    curline++;

//    write( 1, "NLIN: '", 7 );
//    write( 1, lp, i );
//    write( 1, "'\r\n", 3 );

    // Set start of line pointer
    p = lp;

    // Number of bytes read
    return i;
}

#if DBUG_TOK
void ntok_dbg()
#else
void ntok()
#endif
{
    if ( !readline() )
    {
        // End of file
        tk = 0;
        return;
    }

    while ( tk = *p )
    {
        pp = p++;

        if ( tk == '\n' || tk == ';' ) // End of line
        {
            tk = TOK_EOL;
            tkVal = 0;
            return;
        }
        else if ( (tk >= 'a' && tk <= 'z') // Named symbol
               || (tk >= 'A' && tk <= 'Z')
               ||  tk == '_' || tk == '.'  )
        {
            // Get entire token
            while ( p - pp <= MAX_TOKN
             && ( (*p >= 'a' && *p <= 'z')
             ||   (*p >= 'A' && *p <= 'Z')
             ||   (*p >= '0' && *p <= '9')
             ||    *p == '_' || *p == '#') ) p++;

            unsigned char toklen = p - pp;

            // Label excedes max length
            if ( toklen > MAX_TOKN ) asmfail("named token exceeds max character length");

            // Return current segment position
            if ( tk == '.' && toklen == 1 )
            {
                tk = TOK_NUM;
                tkVal = curseg->dataPos;
                return;
            }

            unsigned int fscp = -1;
            int i, k = 0;
            // Find a matching symbol
            while ( k < sympos )
            {
                i = 0;

                // Get all characters that implicitly match TODO case insensitivity
                while ( i < toklen && i < symtbl[k].len && symtbl[k].name[i] == pp[i] ) i++;

                if ( k < ASM_SIZE ) // Assembler defined symbol
                {
                    tk = symtbl[k].type;
                    tkVal = symtbl[k].val;

                    // Exact match
                    if ( i == toklen && toklen == symtbl[k].len )
                    {
                        #if DBUG_SYM
                        write( 1, "ASM SYM MATCH:\r\n", 16 );
                        symwrite( 1, symtbl + k );
                        #endif
                        return;
                    }
                    else if (!( i == symtbl[k].len && toklen > symtbl[k].len )) // No flags
                    {
                        k++;
                        continue;
                    }
                }
                else if ( (symtbl[k].type & SYM_MASK) == SYM_FILE ) // File seperator token
                {
                    fscp++;
                    k++;
                    continue;
                }
                else // User defined symbol
                {
                    if ( i == toklen && toklen == symtbl[k].len // Exact match
                    && (symtbl[k].type & SYM_GLOB || fscp == curfno || k < ASM_SIZE) ) // Correct scope
                    {
                        // Is this a defined value or a symbol
                        tk = (symtbl[k].type & SYM_MASK) == SYM_ABS ? TOK_NUM : TOK_NAME;
                        tkVal = k;
                        #if DBUG_SYM
                        write( 1, "USER SYM MATCH:\r\n", 17 );
                        symwrite( 1, symtbl + k );
                        #endif
                        return;
                    }
                    k++;
                    continue;
                }

                // Number of unmatched chars (flags)
                int flagNum = toklen - i;

                // Check flags if I/O instruction
                if ( flagNum == 1
                && ( tk == DGN_IO
                ||   tk == DGN_CTF
                ||   tk == DGN_CTAF ) )
                {
                    if ( pp[i] == 's' || pp[i] == 'S' ) { tkVal |= 0b0000000001000000; return; }
                    if ( pp[i] == 'c' || pp[i] == 'C' ) { tkVal |= 0b0000000010000000; return; }
                    if ( pp[i] == 'p' || pp[i] == 'P' ) { tkVal |= 0b0000000011000000; return; }
                }
                else if ( tk == DGN_MATH && flagNum >= 1 && flagNum <= 3 )
                {
                    // Carry control
                    if      ( pp[i] == 'z' || pp[i] == 'Z' ) { tkVal |= 0b0000000000010000; i++; flagNum--; }
                    else if ( pp[i] == 'o' || pp[i] == 'O' ) { tkVal |= 0b0000000000100000; i++; flagNum--; }
                    else if ( pp[i] == 'c' || pp[i] == 'C' ) { tkVal |= 0b0000000000110000; i++; flagNum--; }

                    if ( flagNum ) // Shift control
                    {
                        if      ( pp[i] == 'l' || pp[i] == 'L' ) { tkVal |= 0b0000000001000000; i++; flagNum--; }
                        else if ( pp[i] == 'r' || pp[i] == 'R' ) { tkVal |= 0b0000000010000000; i++; flagNum--; }
                        else if ( pp[i] == 's' || pp[i] == 'S' ) { tkVal |= 0b0000000011000000; i++; flagNum--; }
                    }

                    // No-Load
                    if ( flagNum && pp[i] == '#' ) { tkVal |= 0b0000000000001000; flagNum--; }

                    // Exhausted all flags
                    if ( !flagNum )
                    {
                        #if DBUG_SYM
                        write( 1, "SYM MATH MATCH:\r\n", 17 );
                        symwrite( 1, symtbl + k );
                        #endif

                        return;
                    }
                }

                k++;
            }

            // Allocate room for new symbols
            if ( sbrk( sizeof(struct asmsym) ) == SBRKFAIL ) asmfail("cannot allocate room for new symbol");

            // Create new symbol
            i = 0;
            while ( i < toklen ) // Store symbol and fill rest with zeros
            {
                symtbl[k].name[i] = pp[i];
                i++;
            }

            // Null terminate
            if ( i < MAX_TOKN ) symtbl[k].name[i] = 0;

            // Set type to undefiend symbol
            symtbl[k].type = SYM_DEF;
            // Record size of symbol
            symtbl[k].len = toklen;
            // Make this symbol global
            if ( flags & FLG_GLOB ) symtbl[k].type |= SYM_GLOB;
            // Set default value
            symtbl[k].val = 0;

            #if DBUG_SYM
            write( 1, "NEW SYM:\r\n", 10 );
            symwrite( 1, symtbl + k );
            #endif

            sympos++;

            tk = TOK_NAME;
            tkVal = k;
            return;
        }
        else if ( tk >= '0' && tk <= '9' ) // Number
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
                while ( (*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f') || (*p >= 'A' && *p <= 'F') )
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

            tk = TOK_NUM;
            return;
        }
        // Assembler tokens
        else if ( tk == ',' ) { tk = TOK_ARG;  tkVal = 0; return; } // Argument seperator
        else if ( tk == ':' ) { tk = TOK_LABL; tkVal = 0; return; } // Label declaration
        else if ( tk == '@' ) { tk = TOK_INDR; tkVal = 0; return; } // Indirection flag
        else if ( tk == '<' ) { tk = TOK_BYHI; tkVal = 0; return; } // Low  byte pointer flag
        else if ( tk == '>' ) { tk = TOK_BYLO; tkVal = 0; return; } // High byte pointer flag
        else if ( tk == '+' || tk == '-' ) { tk = TOK_MATH; tkVal = tk == '-'; return; } // Plus or minus
        else if ( tk == '"' || tk == '\'' ) // String, double quote is zero terminated
        {
            tkVal = 0;
            usp = ustr;

            while ( *p != tk )
            {
                // Out of data, get next line
                if ( !*p && !readline() ) asmfail("expected terminating string delimiter, got end of file");

                // Escape character
                if ( *p == '\\' )
                {
                    p++;

                    if      ( *p == 'a'  ) { *usp = '\a'; p++; }
                    else if ( *p == 'b'  ) { *usp = '\b'; p++; }
                    else if ( *p == 'e'  ) { *usp = '\e'; p++; }
                    else if ( *p == 'f'  ) { *usp = '\f'; p++; }
                    else if ( *p == 'n'  ) { *usp = '\n'; p++; }
                    else if ( *p == 'r'  ) { *usp = '\r'; p++; }
                    else if ( *p == 't'  ) { *usp = '\t'; p++; }
                    else if ( *p == 'v'  ) { *usp = '\v'; p++; }
                    else if ( *p == '\\' ) { *usp = '\\'; p++; }
                    else if ( *p == '\'' ) { *usp = '\''; p++; }
                    else if ( *p == '"'  ) { *usp = '"';  p++; }
                    else if ( *p == 'x' )
                    {
                        if ( *++p >= '0' && *p <= '9'
                            || *p >= 'a' && *p <= 'f'
                            || *p >= 'A' && *p <= 'F' )
                            // Convert first digit
                            *usp = (*p & 0xF) + (*p++ >= 'A' ? 9 : 0);
                        else asmfail("expected two hex digits following a \\x escape");

                        // Convert possible second digit
                        if ( *p >= '0' && *p <= '9'
                          || *p >= 'a' && *p <= 'f'
                          || *p >= 'A' && *p <= 'F' )
                            *usp = (*usp << 4) + (*p & 0xF) + (*p++ >= 'A' ? 9 : 0);
                    }
                    else if ( *p >= '0' && *p <= '7' )
                    {
                        *usp = *p++ - '0'; // First digit

                        if ( *p >= '0' && *p <= '7' ) // Second digit
                            *usp = (*usp << 3) + *p++ - '0';

                        if ( *p >= '0' && *p <= '7' ) // Last digit
                            *usp = (*usp << 3) + *p++ - '0';
                    }
                    else asmfail("unknown escape sequence");
                }
                else
                {
                    *usp = *p++;
                }

                if ( ++usp - ustr >= MAX_STR ) asmfail("string excedes maximum length");
            }

            p++; // Increment past the final delimiter

            // Null termiante
            if ( tk == '"' ) *usp++ = 0; // Null terminate

            tk = TOK_STR;
            tkVal = usp - ustr; // String length
            return;
        }
    }

    // End of line reached, use end of line token
    tk = TOK_EOL;
    tkVal = 0;
}

#if DBUG_TOK
void ntok()
{
    ntok_dbg();

    write( 1, "INP: '", 6 );
    write( 1, pp, p - pp );
    write( 1, "', TOKEN: ", 10 );
    octwrite( 1, tk );
    write( 1, ", VAL: ", 7 );
    octwrite( 1, tkVal );
    write( 1, "\r\n", 2 );
}
#endif