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

            if ( fpos[2] != '\0' )
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

            if ( fpos[1] != '\0' )
            {
                xlog( DGNASM_LOG_SYTX, state, "Too many flags '%s', valid flags: (None) S, C, P\n", fpos );
                return 0;
            }
        }
        // Set control bits
        data |= control;

        // Check number of arguments
        if ( argc != 2 )
        {
            xlog( DGNASM_LOG_SYTX, state, "Incorrect number of arguments '%d', expected 2\n", argc );
            return 0;
        }

        // Pick an accumulator
        unsigned short accum;
        if ( !convertNumber( argv[0], &accum, 0, 3, state ) ) return 0;
        printf( "Accum: %d\n", accum );
        data |= (accum << 11);

        // Pick a device code
        unsigned short device;
        if ( !convertNumber( argv[1], &device, 0, 63, state ) ) return 0;
        printf( "Device: %d\n", device );
        data |= device;
    }

    // Store instruction to memory
    state->memory[state->curAddr] = data;

    return 1;
}
