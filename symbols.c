struct symbol symtbl[MAX_SYMS] = {
    // I/O Instructions (7)
    { { 'N', 'I', 'O', 0 }, SYM_IONO, 0b0110000000000000 },
    { { 'D', 'I', 'A', 0 }, SYM_IO,   0b0110000100000000 },
    { { 'D', 'O', 'A', 0 }, SYM_IO,   0b0110001000000000 },
    { { 'D', 'I', 'B', 0 }, SYM_IO,   0b0110001100000000 },
    { { 'D', 'O', 'B', 0 }, SYM_IO,   0b0110010000000000 },
    { { 'D', 'I', 'C', 0 }, SYM_IO,   0b0110010100000000 },
    { { 'D', 'O', 'C', 0 }, SYM_IO,   0b0110011000000000 },
    // I/O Skip Instructions (4)
    { { 'S', 'K', 'P', 'B', 'N', 0 }, SYM_IOSK, 0b0110011100000000 },
    { { 'S', 'K', 'P', 'B', 'Z', 0 }, SYM_IOSK, 0b0110011101000000 },
    { { 'S', 'K', 'P', 'D', 'N', 0 }, SYM_IOSK, 0b0110011110000000 },
    { { 'S', 'K', 'P', 'D', 'Z', 0 }, SYM_IOSK, 0b0110011111000000 },
    // Flow control instructions (4)
    { { 'J', 'M', 'P', 0 }, SYM_FLOW, 0b0000000000000000 },
    { { 'J', 'S', 'R', 0 }, SYM_FLOW, 0b0000100000000000 },
    { { 'I', 'S', 'Z', 0 }, SYM_FLOW, 0b0001000000000000 },
    { { 'D', 'S', 'Z', 0 }, SYM_FLOW, 0b0001100000000000 },
    // Memory access instructions (2)
    { { 'L', 'D', 'A', 0 }, SYM_LOAD, 0b0010000000000000 },
    { { 'S', 'T', 'A', 0 }, SYM_LOAD, 0b0100000000000000 },
    // Arithmetic & Logic instructions (8)
    { { 'C', 'O', 'M', 0 }, SYM_MATH, 0b1000000000000000 },
    { { 'N', 'E', 'G', 0 }, SYM_MATH, 0b1000000100000000 },
    { { 'M', 'O', 'V', 0 }, SYM_MATH, 0b1000001000000000 },
    { { 'I', 'N', 'C', 0 }, SYM_MATH, 0b1000001100000000 },
    { { 'A', 'D', 'C', 0 }, SYM_MATH, 0b1000010000000000 },
    { { 'S', 'U', 'B', 0 }, SYM_MATH, 0b1000010100000000 },
    { { 'A', 'D', 'D', 0 }, SYM_MATH, 0b1000011000000000 },
    { { 'A', 'N', 'D', 0 }, SYM_MATH, 0b1000011100000000 },
    // Trap instruction (Arithmetic no-op) (1)
    { { 'T', 'R', 'A', 'P', 0 }, SYM_TRAP, 0b1000000000001000 },
    // Byte acess instructions (2)
    { { 'L', 'D', 'B', 0 }, SYM_CTAA, 0b0110000100000001 },
    { { 'S', 'T', 'B', 0 }, SYM_CTAA, 0b0110010000000001 },
    // Stack instructions (10)
    { { 'P', 'S', 'H', 'A', 0 }, SYM_CTA, 0b0110001100000001 },
    { { 'P', 'S', 'H', 'N', 0 }, SYM_CTA, 0b0110001111000001 }, // [Undocumented] PSHA With I/O Pulse
    { { 'P', 'O', 'P', 'A', 0 }, SYM_CTA, 0b0110001110000001 },
    { { 'M', 'T', 'S', 'P', 0 }, SYM_CTA, 0b0110001000000001 },
    { { 'S', 'A', 'V',      0 }, SYM_CT,  0b0110010100000001 },
    { { 'S', 'A', 'V', 'N', 0 }, SYM_CT,  0b0110010111000001 }, // [Undocumented] SAV with I/O Pulse
    { { 'M', 'T', 'F', 'P', 0 }, SYM_CTA, 0b0110000000000001 },
    { { 'M', 'F', 'S', 'P', 0 }, SYM_CTA, 0b0110001010000001 },
    { { 'M', 'F', 'F', 'P', 0 }, SYM_CTA, 0b0110000010000001 },
    { { 'R', 'E', 'T',      0 }, SYM_CT,  0b0110010110000001 },
    // CPU Control instructions (7)
    { { 'I', 'N', 'T', 'E', 'N', 0 }, SYM_CT,   0b0110000001111111 },
    { { 'I', 'N', 'T', 'D', 'S', 0 }, SYM_CT,   0b0110000010111111 },
    { { 'R', 'E', 'A', 'D', 'S', 0 }, SYM_CTAF, 0b0110000100111111 },
    { { 'M', 'S', 'K', 'O',      0 }, SYM_CTAF, 0b0110010000111111 },
    { { 'I', 'N', 'T', 'A',      0 }, SYM_CTAF, 0b0110001100111111 },
    { { 'I', 'O', 'R', 'S', 'T', 0 }, SYM_CTF,  0b0110010110111111 },
    { { 'H', 'A', 'L', 'T',      0 }, SYM_CTF,  0b0110011000111111 },
    // Hardware Multiply & Divide instructions (4)
    { { 'M', 'U', 'L',      0 }, SYM_CT, 0b0111011011000001 },
    { { 'M', 'U', 'L', 'S', 0 }, SYM_CT, 0b0111111010000001 },
    { { 'D', 'I', 'V',      0 }, SYM_CT, 0b0111011001000001 },
    { { 'D', 'I', 'V', 'S', 0 }, SYM_CT, 0b0111111000000001 },
    // Hardware Floating Point instructions (TODO)

    // Arithmetic & Logic skip conditions (7)
    { { 'S', 'K', 'P', 0 }, SYM_SKPC, 01 },
    { { 'S', 'Z', 'C', 0 }, SYM_SKPC, 02 },
    { { 'S', 'N', 'C', 0 }, SYM_SKPC, 03 },
    { { 'S', 'Z', 'R', 0 }, SYM_SKPC, 04 },
    { { 'S', 'N', 'R', 0 }, SYM_SKPC, 05 },
    { { 'S', 'E', 'Z', 0 }, SYM_SKPC, 06 },
    { { 'S', 'B', 'N', 0 }, SYM_SKPC, 07 },
    // Hardware device aliases (8 so far)
    { { 'M', 'D', 'V',      0 }, SYM_HWID, 001 }, // Multiply & Divide
    { { 'M', 'A', 'P',      0 }, SYM_HWID, 002 }, // Memory Management Unit
    { { 'M', 'A', 'P', '1', 0 }, SYM_HWID, 003 }, // MMU (Takes up two slots)

    { { 'T', 'T', 'I',      0 }, SYM_HWID, 010 }, // TTY input
    { { 'T', 'T', 'O',      0 }, SYM_HWID, 011 }, // TTY output
    { { 'P', 'T', 'R',      0 }, SYM_HWID, 012 }, // Paper tape reader
    { { 'P', 'T', 'P',      0 }, SYM_HWID, 013 }, // Paper tape punch
    { { 'R', 'T', 'C',      0 }, SYM_HWID, 014 }, // Real time clock
};

// Intialized to the total number of symbols in the table above
int cursym = 64;
