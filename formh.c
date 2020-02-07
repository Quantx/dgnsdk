#include "dgnasm.h"

int formatSimh( dgnasm * state )
{
    short blockStart, blockEnd, i;

    // Iterate over entire program
    for ( blockStart = state->startAddr; blockStart < state->curAddr; blockStart++ )
    {
        // Find first non-zero entry
        if ( !state->memory[blockStart] ) continue;

        i = blockStart + 16;
        if ( i > state->curAddr ) i = state->curAddr;

        // Compute end of 20 word block
        for ( blockEnd = i; !state->memory[blockEnd - 1]; blockEnd-- );

        // SimH block header
        short simh[3];

        simh[0] = blockStart - blockEnd; // Negative block length
        simh[1] = blockStart; // Origin of the block
        simh[2] = simh[0] + simh[1]; // Checksum

        // Compute checksum for block
        for ( i = blockStart; i < blockEnd; i++ ) simh[2] += state->memory[i];

        simh[2] = -simh[2]; // Cancel out checksum

        // Output header
        fwrite( &simh, sizeof(short), 3, state->outFile );

        // Output block
        fwrite( state->memory + blockStart, sizeof(short), blockEnd - blockStart, state->outFile );

        blockStart = blockEnd - 1;

        xlog( DGNASM_LOG_DBUG, state, "wrote SimH block: size %d words, origin at [%o], checksum %d\n", -simh[0], simh[1], simh[2] );
    }

    // Specify simh starting address
    short simh[3];
    simh[0] = 1;
    simh[1] = state->entAddr | ((!state->simhStart) << 15);
    simh[2] = 0;

    // Output starting address
    fwrite( &simh, sizeof(short), 3, state->outFile );
}
