#include "dgnasm.h"

int buildInstruction( instr * curIns, char * fpos, int argc, char ** argv, dgnasm * state )
{
    unsigned short data = curIns->opcode;

    // I/O Instruction
    if ( curIns->type == DGNOVA_INSTR_IOC )
    {
        unsigned short control = 0;
        // The SKP instructiono has different flags
        if ( !strcmp( curIns->name, "SKP" ) )
        {
            if ( fpos[0] = 'd' || fpos[0] == 'D' ) control |= 0b10000000;
            else if ( fpos[0] != 'b' && fpos[0] != 'B' )
            {
                xlog( DGNASM_LOG_SYTX, state, "Invalid flags '%s', valid flags: BN, BZ, DN, DZ\n", fpos );
                return 0;
            }

            if ( fpos[1] = 'z' || fpos[1] == 'Z' ) control |= 0b01000000;
            else if ( fpos[1] != 'n' && fpos[1] != 'N' )
            {
                xlog( DGNASM_LOG_SYTX, state, "Invalid flags '%s', valid flags: BN, BZ, DN, DZ\n", fpos );
                return 0;
            }

            if ( strlen( fpos ) > 2 )
            {
                xlog( DGNASM_LOG_SYTX, state, "Too many flags '%s', valid flags: BN, BZ, DN, DZ\n", fpos );
                return 0;
            }
        }
        else
        {
            switch ( *fpos )
            {
                case '\0':
                    // Intentionally empty
                    break;
                case 's':
                case 'S':
                    control = 0b01000000;
                    break;
                case 'c':
                case 'C':
                    control = 0b10000000;
                    break;
                case 'p':
                case 'P':
                    control = 0b11000000;
                    break;
                default:
                    xlog( DGNASM_LOG_SYTX, state, "Invalid flag '%c', valid flags: (None) S, C, P\n", fpos );
                    return 0;
                    break;
            }

            if ( strlen( fpos ) > 1 )
            {
                xlog( DGNASM_LOG_SYTX, state, "Too many flags '%s', valid flags: (None) S, C, P\n", fpos );
                return 0;
            }
        }
        // Set control bits
        data |= control;

        // Check number of arguments
        if ( !checkArgs( argc, 2, 2, state ) ) return 0;

        // Pick an accumulator
        unsigned short accum;
        if ( !convertNumber( argv[0], &accum, 0, 3, state ) ) return 0;
        data |= (accum << 11);

        // Pick a device code
        unsigned short device;
        if ( !convertNumber( argv[1], &device, 0, 63, state ) ) return 0;
        data |= device;
    }
    else if ( curIns->type == DGNOVA_INSTR_CPD )
    {
        // Check number of arguments
        if ( !checkArgs( argc, 1, 1, state ) ) return 0;

        // Pick an accumulator
        unsigned short accum;
        if ( !convertNumber( argv[0], &accum, 0, 3, state ) ) return 0;
        data |= (accum << 11);
    }
    else if ( curIns->type == DGNOVA_INSTR_JMP )
    {
        // Check number of arguments
        if ( !checkArgs( argc, 1, 2, state ) ) return 0;

        char * dispArg = argv[0];

        // Check if paging mode was set
        if ( argc == 2 )
        {
            dispArg = argv[1];

            unsigned short page;
            if ( !convertNumber( argv[0], &page, 0, 3, state ) ) return 0;
            data |= (page << 8);
        }

        // Check for relative offset
        if ( dispArg[0] == '@' )
        {
            // Get real start of displacement
            dispArg++;

            // Set indirect flag
            data |= (1 << 10);
        }

        // Label displacement
        if ( isLabel( dispArg ) )
        {
            insertReference( dispArg, 1, state );
        }
        // Literal displacement
        else
        {
            unsigned short disp;
            if ( !convertNumber( argv[0], &disp, -128, 127, state ) ) return 0;
            data |= disp & 0xFF;
        }
    }
    else if ( curIns->type == DGNOVA_INSTR_MEM )
    {
        // Check number of arguments
        if ( !checkArgs( argc, 1, 3, state ) ) return 0;

        // Pick an accumulator
        unsigned short accum;
        if ( !convertNumber( argv[0], &accum, 0, 3, state ) ) return 0;
        data |= (accum << 11);

        char * dispArg = argv[1];

        // Check if paging mode was set
        if ( argc == 3 )
        {
            dispArg = argv[2];

            unsigned short page;
            if ( !convertNumber( argv[1], &page, 0, 3, state ) ) return 0;
            data |= (page << 8);
        }

        // Check for relative offset
        if ( dispArg[0] == '@' )
        {
            // Get real start of displacement
            dispArg++;

            // Set indirect flag
            data |= (1 << 10);
        }

        // Label displacement
        if ( isLabel( dispArg ) )
        {
            insertReference( dispArg, 1, state );
        }
        // Literal displacement
        else
        {
            unsigned short disp;
            if ( !convertNumber( argv[0], &disp, -128, 127, state ) ) return 0;
            data |= disp & 0xFF;
        }
    }
    else if ( curIns->type == DGNOVA_INSTR_ARL )
    {

    }

    // Store instruction to memory
    state->memory[state->curAddr] = data;

    return 1;
}
