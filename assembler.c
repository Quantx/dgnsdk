void assemble( char * fpath, int pass )
{
    fp = fpath; // Save file path string
    curline = -1; // Reset current line
    p = NULL; // Null out tok pointer for first readline
    fd = open( fpath, 0 ); // Open file for reading

    // Cannot open file
    if ( fd < 0 ) exit(1);

    ntok(); // Get first token and loop until EOF
    while ( tk )
    {
        // Label constant with indirect bit, or high byte, or low byte
        if ( tk == TOK_INDR || tk == TOK_BYLO || tk == TOK_BYHI )
        {
            // Store initial token type
            char lblType = tk;

            ntok();
            if ( tk != TOK_NAME ) exit(1); // The following token MUST be a label

            // Store ref to symbol
            struct symbol * cursym = &symtbl[tkVal];

            // Allocate room for symbol in this segment
            curseg->pos++;

            ntok();
        }
        // Label declaration or label constant
        else if ( tk == TOK_NAME )
        {
            // Store ref to symbol
            struct symbol * cursym = &symtbl[tkVal];

            ntok();
            // Label declaration
            if ( tk == TOK_LABL )
            {
                if ( cursym->type == SYM_DEF ) // Undefined symbol
                {
                    cursym->type = curseg->sym;
                    cursym->val = curseg->pos;
                }
                else // Already defined symbol
                {
                    exit(1);
                }

                ntok();
            }
            // Some other token
            else
            {
                // Allocate room for this symbol in the segment
                curseg->pos++;
            }
        }
        else if ( tk == TOK_NUM ) // Numerical constant
        {
            ntok();

            // Allocate room for absolute symbol in the segment
            curseg->pos++;
        }
        else if ( tk >= DGN_IONO && tk <= DGN_CTAA ) // DGNova instruction
        {
            int optyp = tk; // Type of instruction
            int opval = tkVal; // The 16 bit instruction

            // I/O Instruction, need a device code and maybe an accumulator
            if ( optyp == DGN_IONO || optyp == DGN_IO || optyp == DGN_IOSK )
            {
                if ( optyp == DGN_IO )
                {
                    // Get accumulator
                    ntok();
                    if ( tk != TOK_NUM || tkVal < 0 || tkVal > 3 ) exit(1);
                    opval |= tkVal << 11;
                    // Get comma
                    ntok();
                    if ( tk != TOK_ARG ) exit(1);
                }

                // Get I/O device address (Number of Hardware ID Constant)
                ntok();
                if ( (tk != TOK_NUM && tk != DGN_HWID) || tkVal < 0 || tkVal > 63 ) exit(1);
                opval |= tkVal;

                ntok();
            }
            // Flow Control Instruction, need displacement and maybe a mode
            // Load Instruction, need accumulator, displacement, and maybe a mode
            else if ( optyp == DGN_FLOW || optyp == DGN_LOAD )
            {
                ntok();

                if ( optyp == DGN_LOAD )
                {
                    // Get accumulator
                    if ( tk != TOK_NUM || tkVal < 0 || tkVal > 3 ) exit(1);
                    opval |= tkVal << 11;
                    // Get comma
                    ntok();
                    if ( tk != TOK_ARG ) exit(1);
                    ntok();
                }

                // Get indirect flag
                if ( tk == TOK_INDR )
                {
                    opval |= 0b0000010000000000;
                    ntok();
                }

                // Get displacement
                if ( tk == TOK_NUM ) // Numerical displacement (programmer can pick mode)
                {
                    // Store displacement
                    int disp = tkVal;
                    int mode = 0;

                    // Check if a mode was specified
                    ntok();
                    if ( tk == TOK_ARG )
                    {
                        // Get mode
                        ntok();
                        if ( tk != TOK_NUM || tkVal < 0 || tkVal > 3 ) exit(1);
                        mode = tkVal;

                        ntok();
                    }

                    if ( !mode && (disp < 0 || disp > 0xFF) ) // Zero page access
                    {
                        exit(1);
                    }
                    else if ( mode && (disp < -128 || disp > 127) ) // Two's complement displacement
                    {
                        exit(1);
                    }

                    // Set mode and displacement
                    opval |= mode << 8;
                    opval |= disp;
                }
                else if ( tk == TOK_NAME ) // Label displacement (assembler picks mode)
                {
                    // Store ref to symbol
                    struct symbol * cursym = &symtbl[tkVal];

                    // Zero page symbol
                    if ( cursym->type == SYM_ZERO )
                    {
                        opval |= cursym->val;
                    }
                    // Symbol not in current segment
                    else if ( cursym->type != curseg->sym )
                    {
                        exit(1);
                    }
                    // Program counter relative symbol
                    else
                    {
                        int disp = cursym->val - curseg->pos;

                        // Out of bounds
                        if ( disp < -128 || disp > 127 ) exit(1);

                        opval |= disp;
                    }

                    ntok();
                }
            }
            // Arithmetic & Logic Instructions
            // Need two accumulators and maybe a skip code
            else if ( optyp == DGN_MATH )
            {
                // Get source accumulator
                ntok();
                if ( tk != TOK_NUM || tkVal < 0 || tkVal > 3 ) exit(1);
                opval |= tkVal << 13;

                // Get argument seperator
                ntok();
                if ( tk != TOK_ARG ) exit(1);

                // Get destination accumulator
                ntok();
                if ( tk != TOK_NUM || tkVal < 0 || tkVal > 3 ) exit(1);
                opval |= tkVal << 11;

                // Check if there's a skip code
                ntok();
                if ( tk == TOK_ARG )
                {
                    ntok();

                    // Skip code cosntant
                    if ( tk == DGN_SKPC )
                    {
                        opval |= tkVal;
                    }
                    // Numerical skip code
                    else if ( tk == TOK_NUM && tkVal >= 0 && tkVal <= 7 )
                    {
                        opval |= tkVal;
                    }
                    else
                    {
                        exit(1);
                    }

                    ntok();
                }
                else if ( opval & 8 ) // Don't allow pseudo-trap instructions
                {
                    exit(1);
                }
            }
            // Trap instruction
            // Needs an 11-bit trap number
            else if ( optyp == DGN_TRAP )
            {
                // Ensure this is an 11-bit numerical constant
                ntok();
                if ( tk != TOK_NUM || tk < 0 || tk > 2047 ) exit(1);
                opval |= tkVal < 4;

                ntok();
            }
            // System Control Instruction (CPU, MMU, FPU, etc.)
            else
            {
                if ( optyp == DGN_CTA || optyp == DGN_CTAF || optyp == DGN_CTAA )
                {
                    // Get accumulator
                    ntok();
                    if ( tk != TOK_NUM || tkVal < 0 || tkVal > 3 ) exit(1);
                    opval |= tkVal << 11;

                    if ( optyp == DGN_CTAA )
                    {
                        ntok();
                        if ( tk != TOK_ARG ) exit(1); // Missing argument seperator

                        // Get accumulator
                        ntok();
                        if ( tk != TOK_NUM || tkVal < 0 || tkVal > 3 ) exit(1);
                        opval |= tkVal << 6;
                    }
                }

                ntok();
            }

            // Write out instruction
            curseg->pos++;
        }
        // Assembler .text directive
        else if ( tk == ASM_TEXT ) { curseg = &text; ntok(); }
        // Assembler .data directive
        else if ( tk == ASM_DATA ) { curseg = &data; ntok(); }
        // Assembler  .bss directive
        else if ( tk == ASM_BSS  ) { curseg = &bss;  ntok(); }
        // Assembler .zero directive
        else if ( tk == ASM_ZERO ) { curseg = &zero; ntok(); }
        // Assembler .glob directive
        else if ( tk == ASM_GLOB ) while ( tk == ASM_GLOB || tk == TOK_ARG )
        {
            // Get label
            ntok();
            if ( tk != TOK_NAME ) exit(1);

            // Make label global
            symtbl[tkVal].file = 0;

            ntok(); // Get comma or next statement
        }
        // Assembler .define directive
        else if ( tk == ASM_DEFN )
        {
            // Get label
            ntok();
            if ( tk != TOK_NAME ) exit(1);

            // Store ref to symbol
            struct symbol * cursym = &symtbl[tkVal];

            // Already defined symbol
            if ( cursym->type != SYM_DEF ) exit(1);

            // Get comma
            ntok();
            if ( tk != TOK_ARG ) exit(1);

            // Get numerical value to assign
            ntok();
            if ( tk != TOK_NUM ) exit(1);

            // Assign value to absolute symbol
            cursym->type = SYM_ABS;
            cursym->val = tkVal;
            cursym->file = curfno;
        }
    }
}

