char lp[MAX_LINE], * p, * pp, * fp, tk;
int fd, curline, tkVal;

int readline()
{
    int i = 0;

    // Current line still has data
    if ( p - lp < MAX_LINE || *p ) return -1;

    // Read data in and scan for newline
    while ( i < MAX_LINE - 1 && read(fd, lp + i, 1 ) && lp[i] != '\n' )
    {
        // Termination on comments
        if ( lp[i] == ';' ) lp[i] = 0;
        i++;
    }

    // Buffer overflow
    if ( i == MAX_LINE - 1 ) exit(1);

    // Null termiante
    lp[i] = 0;

    // Increment line count
    curline++;

    // Return number of bytes read
    return i;
}

void ntok()
{
    while ( readline() )
    while ( tk = *p )
    {
        pp = p++;

        if ( (tk >= 'a' && tk <= 'z') // Named symbol
          || (tk >= 'A' && tk <= 'Z')
          ||  tk == '_' )
        {
            // Get entire token
            while ( p - pp < MAX_TOKN
             && ( (*p >= 'a' && *p <= 'z')
             ||   (*p >= 'A' && *p <= 'Z')
             ||   (*p >= '0' && *p <= '9')
             ||    *p == '_' || *p == '#') ) p++;

            // Label excedes max length
            if ( p - pp == MAX_TOKN ) exit(1);

            int i = 0, k = 0;
            // Find a matching symbol
            while ( k < sympos )
            {
                // Get all characters that implicitly match
                while ( i < p - pp && symtbl[k].name[i] == pp[i] ) i++;

                if ( symtbl[k].type <= TOK_HWID ) // Assembler defined symbol
                {
                    tk = symtbl[k].type;
                    tkVal = symtbl[k].val;
                }
                else
                {
                    tk = TOK_NAME;
                    tkVal = k;
                }

                // Exact match
                if ( !symtbl[k].name[i + 1] ) return;

                // Didn't match base name, skip flags
                if ( symtbl[k].name[i] || tk == TOK_NAME ) continue;

                // Number of unmatched chars (flags)
                int flagNum = p - pp - i;

                // Check flags if I/O instruction
                if ( flagNum == 1
                && ( symtbl[k].type == OPC_IO
                ||   symtbl[k].type == OPC_CTF
                ||   symtbl[k].type == OPC_CTAF ) )
                {
                    if ( pp[i] == 's' || pp[i] == 'S' ) { tkVal |= 0b0000000001000000; return; }
                    if ( pp[i] == 'c' || pp[i] == 'C' ) { tkVal |= 0b0000000010000000; return; }
                    if ( pp[i] == 'p' || pp[i] == 'P' ) { tkVal |= 0b0000000011000000; return; }
                }
                else if ( symtbl[k].type = OPC_MATH && flagNum >= 1 && flagNum <= 3 )
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
                    if ( !flagNum ) return;
                }

                k++;
            }


            if ( k == MAX_SYMS ) // Symbol table is full
            {
                exit(1);
            }

            // Create new symbol
            i = 0;
            while ( i < MAX_TOKN ) // Store symbol and fill rest with zeros
            {
                symtbl[k].name[i] = i < p - pp ? pp[i] : 0;
                i++;
            }

            // Set type to undefiend symbol
            symtbl[k].type = flags & FLG_GLOB ? SYM_GDEF : SYM_DEF;
            symtbl[k].val = 0;

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
                    tkVal = (tkVal << 4) + (*p & 15) + (*p++ >= 'A' ? 9 : 0);
            }
            else if ( *p == 'b' || *p == 'B' ) // Binary
            {
                p++;
                while ( *p == '0' || *p == '1' ) tkVal = (tkVal << 1) + *p++ - '0';
            }
            else // Octal
            {
                p++;
                while ( *p >= '0' && *p <= '7' ) tkVal = (tkVal << 3) + *p++ - '0';
            }

            tk = TOK_NUM;
            return;
        }
        // Assembler tokens
        else if ( tk == ',' || tk == ':' || tk == '.' || tk == '@' )
        {
            tkVal = 0;
            return;
        }
        // Math tokens
        else if ( tk == '+' || tk == '-'
               || tk == '*' || tk == '/' || tk == '%'
               || tk == '^' || tk == '|' || tk == '&' || tk == '!'
               || tk == '(' || tk == ')' )
        {
            tkVal = tk;
            tk = TOK_MATH;
            return;
        }
    }

    // End of file reached, return null token
    tk = 0;
}
