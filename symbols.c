#include "dgnasm.h"

struct asmsym symtbl[ASM_SIZE + 1] = {
    // I/O Instructions (7)
    { { 'N', 'I', 'O', 0 }, DGN_IONO, 3, 0b0110000000000000 }, // NIO
    { { 'D', 'I', 'A', 0 }, DGN_IO,   3, 0b0110000100000000 }, // DIA
    { { 'D', 'O', 'A', 0 }, DGN_IO,   3, 0b0110001000000000 }, // DOA
    { { 'D', 'I', 'B', 0 }, DGN_IO,   3, 0b0110001100000000 }, // DIB
    { { 'D', 'O', 'B', 0 }, DGN_IO,   3, 0b0110010000000000 }, // DOB
    { { 'D', 'I', 'C', 0 }, DGN_IO,   3, 0b0110010100000000 }, // DIC
    { { 'D', 'O', 'C', 0 }, DGN_IO,   3, 0b0110011000000000 }, // DOC
    // I/O Skip Instructions (4)
    { { 'S', 'K', 'P', 'B', 'N', 0 }, DGN_IOSK, 5, 0b0110011100000000 }, // SKPBN
    { { 'S', 'K', 'P', 'B', 'Z', 0 }, DGN_IOSK, 5, 0b0110011101000000 }, // SKPBZ
    { { 'S', 'K', 'P', 'D', 'N', 0 }, DGN_IOSK, 5, 0b0110011110000000 }, // SKPDN
    { { 'S', 'K', 'P', 'D', 'Z', 0 }, DGN_IOSK, 5, 0b0110011111000000 }, // SKPDZ
    // Flow control instructions (4)
    { { 'J', 'M', 'P', 0 }, DGN_FLOW, 3, 0b0000000000000000 }, // JMP
    { { 'J', 'S', 'R', 0 }, DGN_FLOW, 3, 0b0000100000000000 }, // JSR
    { { 'I', 'S', 'Z', 0 }, DGN_FLOW, 3, 0b0001000000000000 }, // ISZ
    { { 'D', 'S', 'Z', 0 }, DGN_FLOW, 3, 0b0001100000000000 }, // DSZ
    // Memory access instructions (2)
    { { 'L', 'D', 'A', 0 }, DGN_LOAD, 3, 0b0010000000000000 }, // LDA
    { { 'S', 'T', 'A', 0 }, DGN_LOAD, 3, 0b0100000000000000 }, // STA
    // Arithmetic & Logic instructions (8)
    { { 'C', 'O', 'M', 0 }, DGN_MATH, 3, 0b1000000000000000 }, // COM
    { { 'N', 'E', 'G', 0 }, DGN_MATH, 3, 0b1000000100000000 }, // NEG
    { { 'M', 'O', 'V', 0 }, DGN_MATH, 3, 0b1000001000000000 }, // MOV
    { { 'I', 'N', 'C', 0 }, DGN_MATH, 3, 0b1000001100000000 }, // INC
    { { 'A', 'D', 'C', 0 }, DGN_MATH, 3, 0b1000010000000000 }, // ADC
    { { 'S', 'U', 'B', 0 }, DGN_MATH, 3, 0b1000010100000000 }, // SUB
    { { 'A', 'D', 'D', 0 }, DGN_MATH, 3, 0b1000011000000000 }, // ADD
    { { 'A', 'N', 'D', 0 }, DGN_MATH, 3, 0b1000011100000000 }, // AND
    // Trap instruction (Arithmetic no-op) (1)
    { { 'T', 'R', 'A', 'P', 0 }, DGN_TRAP, 4, 0b1000000000001000 }, // TRAP
    // Byte acess instructions (2)
    { { 'L', 'D', 'B', 0 }, DGN_CTAA, 3, 0b0110000100000001 }, // LDB
    { { 'S', 'T', 'B', 0 }, DGN_CTAA, 3, 0b0110010000000001 }, // STB
    // Stack instructions (10)
    { { 'P', 'S', 'H', 'A', 0 }, DGN_CTA, 4, 0b0110001100000001 }, // PSHA
    { { 'P', 'S', 'H', 'N', 0 }, DGN_CTA, 4, 0b0110001111000001 }, // PSHN - [Undocumented] PSHA With I/O Pulse
    { { 'P', 'O', 'P', 'A', 0 }, DGN_CTA, 4, 0b0110001110000001 }, // POPA
    { { 'S', 'A', 'V',      0 }, DGN_CT,  3, 0b0110010100000001 }, // SAV
    { { 'S', 'A', 'V', 'N', 0 }, DGN_CT,  4, 0b0110010111000001 }, // SAVN - [Undocumented] SAV with I/O Pulse
    { { 'R', 'E', 'T',      0 }, DGN_CT,  3, 0b0110010110000001 }, // RET
    { { 'M', 'T', 'S', 'P', 0 }, DGN_CTA, 4, 0b0110001000000001 }, // MTSP
    { { 'M', 'T', 'F', 'P', 0 }, DGN_CTA, 4, 0b0110000000000001 }, // MTFP
    { { 'M', 'F', 'S', 'P', 0 }, DGN_CTA, 4, 0b0110001010000001 }, // MFSP
    { { 'M', 'F', 'F', 'P', 0 }, DGN_CTA, 4, 0b0110000010000001 }, // MFFP
    // CPU Control instructions (7)
    { { 'I', 'N', 'T', 'E', 'N', 0 }, DGN_CT,   5, 0b0110000001111111 }, // INTEN
    { { 'I', 'N', 'T', 'D', 'S', 0 }, DGN_CT,   5, 0b0110000010111111 }, // INTDS
    { { 'R', 'E', 'A', 'D', 'S', 0 }, DGN_CTAF, 5, 0b0110000100111111 }, // READS
    { { 'M', 'S', 'K', 'O',      0 }, DGN_CTAF, 4, 0b0110010000111111 }, // MSKO
    { { 'I', 'N', 'T', 'A',      0 }, DGN_CTAF, 4, 0b0110001100111111 }, // INTA
    { { 'I', 'O', 'R', 'S', 'T', 0 }, DGN_CTF,  5, 0b0110010110111111 }, // IORST
    { { 'H', 'A', 'L', 'T',      0 }, DGN_CTF,  4, 0b0110011000111111 }, // HALT
    // Hardware Multiply & Divide instructions (4)
    { { 'M', 'U', 'L',      0 }, DGN_CT, 3, 0b0111011011000001 }, // MUL
    { { 'M', 'U', 'L', 'S', 0 }, DGN_CT, 4, 0b0111111010000001 }, // MULS
    { { 'D', 'I', 'V',      0 }, DGN_CT, 3, 0b0111011001000001 }, // DIV
    { { 'D', 'I', 'V', 'S', 0 }, DGN_CT, 4, 0b0111111000000001 }, // DIVS
    // Hardware Floating Point instructions (TODO)

    // Arithmetic & Logic skip conditions (7)
    { { 'S', 'K', 'P', 0 }, DGN_SKPC, 3, 01 }, // SKP
    { { 'S', 'Z', 'C', 0 }, DGN_SKPC, 3, 02 }, // SZC
    { { 'S', 'N', 'C', 0 }, DGN_SKPC, 3, 03 }, // SNC
    { { 'S', 'Z', 'R', 0 }, DGN_SKPC, 3, 04 }, // SZR
    { { 'S', 'N', 'R', 0 }, DGN_SKPC, 3, 05 }, // SNR
    { { 'S', 'E', 'Z', 0 }, DGN_SKPC, 3, 06 }, // SEZ
    { { 'S', 'B', 'N', 0 }, DGN_SKPC, 3, 07 }, // SBN
    // Assembler directives (6)
    { { '.', 'T', 'E', 'X', 'T',           0 }, ASM_TEXT, 5, 0 }, // .TEXT
    { { '.', 'D', 'A', 'T', 'A',           0 }, ASM_DATA, 5, 0 }, // .DATA
    { { '.', 'B', 'S', 'S',                0 }, ASM_BSS,  4, 0 }, // .BSS
    { { '.', 'Z', 'E', 'R', 'O',           0 }, ASM_ZERO, 5, 0 }, // .ZERO
    { { '.', 'G', 'L', 'O', 'B',           0 }, ASM_GLOB, 5, 0 }, // .GLOB
    { { '.', 'D', 'E', 'F', 'I', 'N', 'E', 0 }, ASM_DEFN, 7, 0 }, // .DEFINE
    { { '.', 'E', 'N', 'T',                0 }, ASM_ENT,  4, 0 }, // .ENT
    { { '.', 'W', 'S', 'T', 'R',           0 }, ASM_WSTR, 5, 0 }, // .WSTR (Word String)
    // Hardware device aliases (8 so far)
    { { 'M', 'D', 'V',      0 }, DGN_HWID, 3, 001 }, // MDV - Multiply & Divide
    { { 'M', 'A', 'P',      0 }, DGN_HWID, 3, 002 }, // MAP  - Memory Management Unit
    { { 'M', 'A', 'P', '1', 0 }, DGN_HWID, 4, 003 }, // MAP1 - Memory Management Unit (Takes up two slots)

    { { 'T', 'T', 'I',      0 }, DGN_HWID, 3, 010 }, // TTI - TTY input
    { { 'T', 'T', 'O',      0 }, DGN_HWID, 3, 011 }, // TTO - TTY output
    { { 'P', 'T', 'R',      0 }, DGN_HWID, 3, 012 }, // PTR - Paper tape reader
    { { 'P', 'T', 'P',      0 }, DGN_HWID, 3, 013 }, // PTP - Paper tape punch
    { { 'R', 'T', 'C',      0 }, DGN_HWID, 3, 014 }, // RTC - Real time clock

    { { 0 }, 0xFF, 0, 0 } // MUST BE LAST
};

int main( int argc, char ** argv )
{
    // Open symbols file for output
    int ofd = creat( "symbols.dat", 0444 );

    // Make sure we actually created the file
    if ( ofd < 0 )
    {
        write( 2, "Failed to create file symbols.dat\r\n", 35 );
        return 1;
    }
    // Verify table size
    else if ( symtbl[ASM_SIZE].type != 0xFF )
    {
        write( 2, "Symbol table has incorrect size\r\n", 33 );
        return 1;
    }

    // Write data structure to file
    write( ofd, symtbl, ASM_SIZE * sizeof(struct asmsym) );

    close( ofd );
    return 0;
}
