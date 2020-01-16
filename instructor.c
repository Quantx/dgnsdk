#include "dgnasm.h"

int buildInstruction( instr * curIns, char * fpos, int argc, char ** argv, dgnasm * state )
{
    int i;

    unsigned short data = curIns->opcode;

    // I/O Instruction
    if ( curIns->type == DGNOVA_INSTR_IOC )
    {
        unsigned short control = 0;
        int flagError = 0;
        // The SKP instructiono has different flags
        if ( !strcmp( curIns->name, "SKP" ) )
        {
            int flagDone = -1;
            int flagBusy = -1;

            for ( i = 0; fpos[i] != '\0'; i++ )
            {
                if ( i >= 2 )
                {
                    flagError = 1;
                    break;
                }

                if      ( fpos[i] == 'b' || fpos[i] == 'B' ) flagDone = 0;
                else if ( fpos[i] == 'd' || fpos[i] == 'D' ) flagDone = 1;
                else if ( fpos[i] == 'n' || fpos[i] == 'N' ) flagBusy = 0;
                else if ( fpos[i] == 'z' || fpos[i] == 'Z' ) flagBusy = 1;
            }

            if ( flagError || flagDone < 0 || flagBusy < 0 )
            {
                xlog( DGNASM_LOG_SYTX, state, "Invalid flags '%s', valid flags: BN, BZ, DN, DZ\n", fpos );
                return 0;
            }

            if ( flagDone ) control |= 1 << 8;
            if ( flagBusy ) control |= 1 << 7;
        }
        else
        {
            if      ( fpos[0] == 's' || fpos[0] == 'S' ) control = 1;
            else if ( fpos[0] == 'c' || fpos[0] == 'C' ) control = 2;
            else if ( fpos[0] == 'p' || fpos[0] == 'P' ) control = 3;
            else if ( fpos[0] != '\0' ) flagError = 1;

            if ( strlen( fpos ) > 1 ) flagError = 1;

            if ( flagError )
            {
                xlog( DGNASM_LOG_SYTX, state, "Invalid flags '%s', valid flags: (None) S, C, P\n", fpos );
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

        unsigned short page = 0;
        // Check if paging mode was set
        if ( argc == 2 )
        {
            if ( !convertNumber( argv[1], &page, 0, 3, state ) ) return 0;
            data |= (page << 8);
        }

        // Check for relative offset
        if ( *(argv[0]) == '@' )
        {
            // Get real start of displacement
            argv[0]++;

            // Set indirect flag
            data |= (1 << 10);
        }

        unsigned short disp = 0;
        // Label displacement
        if ( isLabel( argv[0] ) )
        {
            unsigned short baseAddr = 0;
            if      ( page == 0 ) baseAddr = 0;
            else if ( page == 1 ) baseAddr = state->curAddr;
            else
            {
                xlog( DGNASM_LOG_SYTX, state, "cannot use page mode %d with a label\n", page );
                return 0;
            }

            if ( !insertReference( argv[0], &disp, baseAddr, state ) ) return 0;
        }
        // Literal displacement
        else
        {
            if ( !convertNumber( argv[0], &disp, 1, 0xFF, state ) ) return 0;
        }

        data |= disp;
    }
    else if ( curIns->type == DGNOVA_INSTR_MEM )
    {
        // Check number of arguments
        if ( !checkArgs( argc, 1, 3, state ) ) return 0;

        // Pick an accumulator
        unsigned short accum;
        if ( !convertNumber( argv[0], &accum, 0, 3, state ) ) return 0;
        data |= (accum << 11);

        unsigned short page = 0;
        // Check if paging mode was set
        if ( argc == 3 )
        {
            if ( !convertNumber( argv[2], &page, 0, 3, state ) ) return 0;
            data |= (page << 8);
        }

        // Check for relative offset
        if ( *(argv[1]) == '@' )
        {
            // Get real start of displacement
            argv[1]++;

            // Set indirect flag
            data |= (1 << 10);
        }

        unsigned short disp = 0;
        // Label displacement
        if ( isLabel( argv[1] ) )
        {
            unsigned short baseAddr = 0;
            if      ( page == 0 ) baseAddr = 0;
            else if ( page == 1 ) baseAddr = state->curAddr;
            else
            {
                xlog( DGNASM_LOG_SYTX, state, "cannot use page mode %d with a label\n", page );
                return 0;
            }

            if ( !insertReference( argv[1], &disp, baseAddr, state ) ) return 0;
        }
        // Literal displacement
        else
        {
            if ( !convertNumber( argv[1], &disp, 1, 0xFF, state ) ) return 0;
        }
        data |= disp;
    }
    else if ( curIns->type == DGNOVA_INSTR_ARL )
    {
        // Check number of arguments
        if ( !checkArgs( argc, 2, 3, state ) ) return 0;

        int flagError = 0;
        unsigned short flagShift = 0;
        unsigned short flagCarry = 0;

        // Decode flags
        for ( i = 0; fpos[i] != '\0' && fpos[i] != '#'; i++ )
        {
            if      ( fpos[i] == 'z' || fpos[i] == 'Z' ) flagCarry = 1;
            else if ( fpos[i] == 'o' || fpos[i] == 'O' ) flagCarry = 2;
            else if ( fpos[i] == 'c' || fpos[i] == 'C' ) flagCarry = 3;

            else if ( fpos[i] == 'l' || fpos[i] == 'L' ) flagShift = 1;
            else if ( fpos[i] == 'r' || fpos[i] == 'R' ) flagShift = 2;
            else if ( fpos[i] == 's' || fpos[i] == 'S' ) flagShift = 3;
            else
            {
                flagError = 1;
                break;
            }

            if ( i >= 2 )
            {
                flagError = 1;
                break;
            }
        }

        if ( flagError )
        {
            xlog( DGNASM_LOG_SYTX, state, "Invalid flags '%s', valid flags: (None)/Z/O/C, (NONE)/L/R/S, (None)/#\n", fpos );
            return 0;
        }

        // Set flags
        data |= flagCarry << 4;
        data |= flagShift << 6;

        // No load flag
        if ( fpos[i] == '#' )
        {
            data |= 1 << 3;

            if ( fpos[i + 1] != '\0' )
            {
                xlog( DGNASM_LOG_SYTX, state, "Invalid flags '%s', the flag '#' must be last\n", fpos );
                return 0;
            }
        }

        // Pick a source accumulator
        unsigned short sac;
        if ( !convertNumber( argv[0], &sac, 0, 3, state ) ) return 0;
        data |= (sac << 13);

        // Pick a destination accumulator
        unsigned short dac;
        if ( !convertNumber( argv[1], &dac, 0, 3, state ) ) return 0;
        data |= (dac << 11);

        // Skip instruction specified
        if ( argc == 3 )
        {
            unsigned short k;
            for ( k = 0; k < 7; k++ )
            {
                if ( !dgnasmcmp( argv[2], skipCodes[k], state ) ) break;
            }

            data |= k + 1;
        }
    }

    // Store instruction to memory
    state->memory[state->curAddr] = data;

    return 1;
}
