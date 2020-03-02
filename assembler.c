void pass1( char * fpath )
{
    fp = fpath; // Save file path string
    curline = -1; // Reset current line
    fd = open( fpath, 0 ); // Open file for reading

    // Cannot open file
    if ( fd < 0 ) exit(1);

    ntok(); // Get first token and loop until EOF
    while ( tk )
    {
        // Label declaration or label constant
        if ( tk == TOK_NAME )
        {
            // Store symbol index
            struct symbol * cursym = &symtbl[tkVal];

            // Get next token
            ntok();
            // Label declaration
            if ( tk == ':' )
            {
                if ( cursym->type == SYM_DEF ) // Local symbol
                {
                    cursym->type = curseg->lsym;
                    cursym->val = curseg->pos;
                }
                else if ( cursym->type == SYM_GDEF ) // Global symbol
                {
                    cursym->type = curseg->gsym;
                    cursym->val = curseg->pos;
                }
                else // Already defined symbol
                {
                    exit(1);
                }
            }
            // Math token, evaluate expression
            else if ( tk == TOK_MATH )
            {
                // Symbol not defined
                if ( cursym->type == SYM_DEF || cursym->type == SYM_GDEF )
                {
                    exit(1);
                }

                expr( cursym->val );
            }
            // Some other token
            else
            {
                
            }
        }
        else if ( tk == TOK_NUM ) // Numerical constant
        {
            int val = tkVal;

            ntok();
            if ( tk == TOK_MATH )
            {
                val = expr( cursym->val );
            }
        }
    }
}
