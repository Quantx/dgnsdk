#include "dgnasm.h"

void initOpcodeTable()
{
    // *** I/O Instructions *** //

    // No I/O transfer
    dgnlang[ 0].name = "NIO";
    dgnlang[ 0].opcode = 0b0110000000000000;
    dgnlang[ 0].type = DGNOVA_INSTR_IOC;

    // Data in  from buffer A
    dgnlang[ 1].name = "DIA";
    dgnlang[ 1].opcode = 0b0110000100000000;
    dgnlang[ 1].type = DGNOVA_INSTR_IOC;

    // Data out from buffer A
    dgnlang[ 2].name = "DOA";
    dgnlang[ 2].opcode = 0b0110001000000000;
    dgnlang[ 2].type = DGNOVA_INSTR_IOC;

    // Data in  from buffer B
    dgnlang[ 3].name = "DIB";
    dgnlang[ 3].opcode = 0b0110001100000000;
    dgnlang[ 3].type = DGNOVA_INSTR_IOC;

    // Data out from buffer B
    dgnlang[ 4].name = "DOB";
    dgnlang[ 4].opcode = 0b0110010000000000;
    dgnlang[ 4].type = DGNOVA_INSTR_IOC;

    // Data in  from buffer C
    dgnlang[ 5].name = "DIC";
    dgnlang[ 5].opcode = 0b0110010100000000;
    dgnlang[ 5].type = DGNOVA_INSTR_IOC;

    // Data out from buffer C
    dgnlang[ 6].name = "DOC";
    dgnlang[ 6].opcode = 0b0110011000000000;
    dgnlang[ 6].type = DGNOVA_INSTR_IOC;

    // Skip on condition
    dgnlang[ 7].name = "SKP";
    dgnlang[ 7].opcode = 0b0110011100000000;
    dgnlang[ 7].type = DGNOVA_INSTR_IOC;

    // *** Jump Instructions *** //

    // Jump to effective address
    dgnlang[ 8].name = "JMP";
    dgnlang[ 8].opcode = 0b0000000000000000;
    dgnlang[ 8].type = DGNOVA_INSTR_JMP;

    // Jump to subroutine
    dgnlang[ 9].name = "JSR";
    dgnlang[ 9].opcode = 0b0000100000000000;
    dgnlang[ 9].type = DGNOVA_INSTR_JMP;

    // Increment and skip if zero
    dgnlang[10].name = "ISZ";
    dgnlang[10].opcode = 0b0001000000000000;
    dgnlang[10].type = DGNOVA_INSTR_JMP;

    // Decrement and skip if zero
    dgnlang[11].name = "DSZ";
    dgnlang[11].opcode = 0b0001100000000000;
    dgnlang[11].type = DGNOVA_INSTR_JMP;

    // *** Memory Instructions *** //

    // Load accumulator
    dgnlang[12].name = "LDA";
    dgnlang[12].opcode = 0b0010000000000000;
    dgnlang[12].type = DGNOVA_INSTR_MEM;

    // Store accumulator
    dgnlang[13].name = "STA";
    dgnlang[13].opcode = 0b0100000000000000;
    dgnlang[13].type = DGNOVA_INSTR_MEM;

    // *** Arithmetic & Logic Instructions *** //

    // Complement value
    dgnlang[14].name = "COM";
    dgnlang[14].opcode = 0b1000000000000000;
    dgnlang[14].type = DGNOVA_INSTR_ARL;

    // Negate a 2s complement value
    dgnlang[15].name = "NEG";
    dgnlang[15].opcode = 0b1000000100000000;
    dgnlang[15].type = DGNOVA_INSTR_ARL;

    // Move a value
    dgnlang[16].name = "MOV";
    dgnlang[16].opcode = 0b1000001000000000;
    dgnlang[16].type = DGNOVA_INSTR_ARL;

    // Increment a value
    dgnlang[17].name = "INC";
    dgnlang[17].opcode = 0b1000001100000000;
    dgnlang[17].type = DGNOVA_INSTR_ARL;

    // Add a 1s complement value
    dgnlang[18].name = "ADC";
    dgnlang[18].opcode = 0b1000010000000000;
    dgnlang[18].type = DGNOVA_INSTR_ARL;

    // Subtract a 2s complement value
    dgnlang[19].name = "SUB";
    dgnlang[19].opcode = 0b1000010100000000;
    dgnlang[19].type = DGNOVA_INSTR_ARL;

    // Add a 2s complement value
    dgnlang[20].name = "ADD";
    dgnlang[20].opcode = 0b1000011000000000;
    dgnlang[20].type = DGNOVA_INSTR_ARL;

    // Logical AND a value
    dgnlang[21].name = "AND";
    dgnlang[21].opcode = 0b1000011100000000;
    dgnlang[21].type = DGNOVA_INSTR_ARL;

    // *** CPU Instructions *** //

    // Reset all I/O devices
    dgnlang[22].name = "IORST";
    dgnlang[22].opcode = 0b0110010110111111;
    dgnlang[22].type = DGNOVA_INSTR_CPC;

    // Halt the CPU
    dgnlang[23].name = "HALT";
    dgnlang[23].opcode = 0b0110011000111111;
    dgnlang[23].type = DGNOVA_INSTR_CPC;

    // Read frontpannel switches
    dgnlang[24].name = "READS";
    dgnlang[24].opcode = 0b0110000100111111;
    dgnlang[24].type = DGNOVA_INSTR_CPD;

    // Interrupt enable
    dgnlang[25].name = "INTEN";
    dgnlang[25].opcode = 0b0110000001111111;
    dgnlang[25].type = DGNOVA_INSTR_CPC;

    // Interrupt disable
    dgnlang[26].name = "INTDS";
    dgnlang[26].opcode = 0b0110000010111111;
    dgnlang[26].type = DGNOVA_INSTR_CPC;

    // Interrupt mask out
    dgnlang[27].name = "MSKO";
    dgnlang[27].opcode = 0b0110010000111111;
    dgnlang[27].type = DGNOVA_INSTR_CPD;

    // Interrupt acknowledge
    dgnlang[28].name = "INTA";
    dgnlang[28].opcode = 0b0110001100111111;
    dgnlang[28].type = DGNOVA_INSTR_CPD;

    // Compute length of each mnemonic
    int i;
    for ( i = 0; i < DGNOVA_LANG_LEN; i++ )
    {
        dgnlang[i].len = strlen( dgnlang[i].name );
    }

    // Load skip codes
    skipCodes[0] = "SKP"; // Skip unconditionally
    skipCodes[1] = "SZC"; // Skip if carry equal zero
    skipCodes[2] = "SNC"; // Skip if carry not equal zero
    skipCodes[3] = "SZR"; // Skip if result if result of operation is zero
    skipCodes[4] = "SNR"; // Skip if result if result of operation is non zero
    skipCodes[5] = "SEZ"; // Skip if either carry or result are zero (or both)
    skipCodes[6] = "SBN"; // Skip if both carry and result are non zero
}

instr * getOpcode( char * mnem, dgnasm * state )
{
    int i;

    // Find matching opcode
    for ( i = 0; i < DGNOVA_LANG_LEN; i++ )
    {
        if ( !dgnasmncmp( mnem, dgnlang[i].name, dgnlang[i].len, state ) )
        {
            return &dgnlang[i];
        }
    }

    return NULL;
}
