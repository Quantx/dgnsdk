// Store current line, keep track of current position
char lp[MAX_LINE], * p, * pp, tk;
int curline = -1, tkVal;

int readline( )
{
    int i = 0;

    // Current line still has data
    if ( p - lp < MAX_LINE || *p ) return -1;

    // Read data in and scan for newline
    while ( i < MAX_LINE - 1 && read(fd + i, lp, 1 ) && lp[i] != '\n' ) i++;

    // Buffer overflow
    if ( i == MAX_LINE ) exit(1);

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

        if ( tk == '\n' ) // End of line
        {
            break;
        }
        else if ( tk == ';' ) // Rest of line is comment
        {
            break;
        }
        else if ( (tk >= 'a' && tk <= 'z') // Named symbol
               || (tk >= 'A' && tk <= 'Z')
               ||  tk == '_' )
        {
            // Get entire token
            while ( p - pp < MAX_TOKN
                 && ( (*p >= 'a' && *p <= 'z')
                 ||   (*p >= 'A' && *p <= 'Z')
                 ||   (*p >= '0' && *p <= '9')
                 ||    *p == '_'               ) ) p++;

            // Label excedes max length
            if ( p - pp == MAX_TOKN ) exit(1);

            // Find a matching symbol
            tkVal = 0;
            while ( tkVal < cursym )
            {
                int i = 0;
                while ( i < MAX_TOKN && !symtbl[tkVal].name[i] ) i++;

                if ( !datcmp( symtbl[tkVal].name, pp, i ) ) break;

                tkVal++;
            }

            if ( tkVal == MAX_SYMS )
            {
                exit(1);
            }
            else if ( tkVal == cursym )
            {
                datcpy( symtbl[tkVal].name, pp, p - pp );
                symtbl[tkVal].type = 0;
                symtbl[tkVal].val = 0;

                cursym++;
            }

            tk = TOK_NAME;
            return;
        }
        else if ( tk >= '0' && tk <= '9' ) // Number
        {
            tkVal = 0;

            if ( tk != '0' ) // Decimal value
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
        else if ( tk == ',' ) // Seperator token
        {
            tkVal = 0;
            tk = TOK_SEP;
            return;
        }
    }
}
