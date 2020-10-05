struct asmsym * symtbl = NULL, ** symtail;

struct instruct insts[] = {
    // I/O Instructions (7)
    { "NIO", 3, DGN_IONO, 0b0110000000000000, CPU_BASE, 0 },
    { "DIA", 3, DGN_IO,   0b0110000100000000, CPU_BASE, 0 },
    { "DOA", 3, DGN_IO,   0b0110001000000000, CPU_BASE, 0 },
    { "DIB", 3, DGN_IO,   0b0110001100000000, CPU_BASE, 0 },
    { "DOB", 3, DGN_IO,   0b0110010000000000, CPU_BASE, 0 },
    { "DIC", 3, DGN_IO,   0b0110010100000000, CPU_BASE, 0 },
    { "DOC", 3, DGN_IO,   0b0110011000000000, CPU_BASE, 0 },
    // I/O Skip instructions (4)
    { "SKPBN", 5, DGN_IOSK, 0b0110011100000000, CPU_BASE, 0 },
    { "SKPBZ", 5, DGN_IOSK, 0b0110011101000000, CPU_BASE, 0 },
    { "SKPDN", 5, DGN_IOSK, 0b0110011110000000, CPU_BASE, 0 },
    { "SKPDZ", 5, DGN_IOSK, 0b0110011111000000, CPU_BASE, 0 },
    // Flow control instructions (4)
    { "JMP", 3, DGN_FLOW, 0b0000000000000000, CPU_BASE, 0 },
    { "JSR", 3, DGN_FLOW, 0b0000100000000000, CPU_BASE, 0 },
    { "ISZ", 3, DGN_FLOW, 0b0001000000000000, CPU_BASE, 0 },
    { "DSZ", 3, DGN_FLOW, 0b0001100000000000, CPU_BASE, 0 },
    // Memory access instructions (2)
    { "LDA", 3, DGN_LOAD, 0b0010000000000000, CPU_BASE, 0 },
    { "STA", 3, DGN_LOAD, 0b0100000000000000, CPU_BASE, 0 },
    // Arithmetic & Logic instructions (8)
    { "COM", 3, DGN_MATH, 0b1000000000000000, CPU_BASE, 0 },
    { "NEG", 3, DGN_MATH, 0b1000000100000000, CPU_BASE, 0 },
    { "MOV", 3, DGN_MATH, 0b1000001000000000, CPU_BASE, 0 },
    { "INC", 3, DGN_MATH, 0b1000001100000000, CPU_BASE, 0 },
    { "ADC", 3, DGN_MATH, 0b1000010000000000, CPU_BASE, 0 },
    { "SUB", 3, DGN_MATH, 0b1000010100000000, CPU_BASE, 0 },
    { "ADD", 3, DGN_MATH, 0b1000011000000000, CPU_BASE, 0 },
    { "AND", 3, DGN_MATH, 0b1000011100000000, CPU_BASE, 0 },
    // Arithmetic & Logic skip conditions (7)
    { "SKP", 3, DGN_SKPC, 01, CPU_BASE, 0 },
    { "SZC", 3, DGN_SKPC, 02, CPU_BASE, 0 },
    { "SNC", 3, DGN_SKPC, 03, CPU_BASE, 0 },
    { "SZR", 3, DGN_SKPC, 04, CPU_BASE, 0 },
    { "SNR", 3, DGN_SKPC, 05, CPU_BASE, 0 },
    { "SEZ", 3, DGN_SKPC, 06, CPU_BASE, 0 },
    { "SBN", 3, DGN_SKPC, 07, CPU_BASE, 0 },
    // Trap instruction (Arithmetic no-op) (1)
    { "TRAP", 4, DGN_TRAP, 0b1000000000001000, CPU_NOVA3 | CPU_NOVA4 | CPU_F9445 | CPU_NOEMU, 0 },
    // Byte acess instructions (2)
    { "LDB", 3, DGN_CTAA, 0b0110000100000001, CPU_NOVA4 | CPU_F9445, 11 },
    { "STB", 3, DGN_CTAA, 0b0110010000000001, CPU_NOVA4 | CPU_F9445, 12 },
    // Stack instructions (10)
    { "PSHA", 4, DGN_CTA, 0b0110001100000001, CPU_NOVA4 | CPU_NOVA3 | CPU_F9445, 3 },
    { "PSHN", 4, DGN_CTA, 0b0110001111000001, CPU_NOVA4 | CPU_NOEMU, 0 }, // [Undocumented] PSHA With I/O Pulse
    { "POPA", 4, DGN_CTA, 0b0110001110000001, CPU_NOVA4 | CPU_NOVA3 | CPU_F9445, 4 },
    { "SAV",  3, DGN_CT,  0b0110010100000001, CPU_NOVA4 | CPU_NOVA3 | CPU_F9445, 5 },
    { "SAVN", 4, DGN_CT,  0b0110010111000001, CPU_NOVA4 | CPU_NOEMU, 0 }, // [Undocumented] SAV with I/O Pulse
    { "RET",  3, DGN_CT,  0b0110010110000001, CPU_NOVA4 | CPU_NOVA3 | CPU_F9445, 6 },
    { "MTSP", 4, DGN_CTA, 0b0110001000000001, CPU_NOVA4 | CPU_NOVA3 | CPU_F9445, 7 },
    { "MTFP", 4, DGN_CTA, 0b0110000000000001, CPU_NOVA4 | CPU_NOVA3 | CPU_F9445, 8 },
    { "MFSP", 4, DGN_CTA, 0b0110001010000001, CPU_NOVA4 | CPU_NOVA3 | CPU_F9445, 9 },
    { "MFFP", 4, DGN_CTA, 0b0110000010000001, CPU_NOVA4 | CPU_NOVA3 | CPU_F9445, 10 },
    // CPU Control instructions (7)
    { "INTEN", 5, DGN_CT,   0b0110000001111111, CPU_BASE, 0 },
    { "INTDS", 5, DGN_CT,   0b0110000010111111, CPU_BASE, 0 },
    { "READS", 5, DGN_CTAF, 0b0110000100111111, CPU_BASE, 0 },
    { "MSKO",  4, DGN_CTAF, 0b0110010000111111, CPU_BASE, 0 },
    { "INTA",  4, DGN_CTAF, 0b0110001100111111, CPU_BASE, 0 },
    { "IORST", 5, DGN_CTF,  0b0110010110111111, CPU_BASE, 0 },
    { "HALT",  4, DGN_CTF,  0b0110011000111111, CPU_BASE, 0 },
    // Hardware Multiply & Divide instructions (4)
    { "MUL",  3, DGN_CT, 0b0111011011000001, CPU_MDV | CPU_NOVA3 | CPU_NOVA4 | CPU_F9445, 1 },
    { "MULS", 4, DGN_CT, 0b0111111010000001, CPU_NOVA4 | CPU_F9445, 13 },
    { "DIV",  3, DGN_CT, 0b0111011001000001, CPU_MDV | CPU_NOVA3 | CPU_NOVA4 | CPU_F9445, 2 },
    { "DIVS", 4, DGN_CT, 0b0111111000000001, CPU_NOVA4 | CPU_F9445, 14 },
    // Virtual Instructions (3)
    { "SYSCALL", 7, DGN_NULL, 0, CPU_VINST, 0 }, // Used for OS calls on Novix
    { "SIGN",    4, DGN_NULL, 0, CPU_VINST, 25 }, // Accumulator sign extension
    { "XOR",     3, DGN_NULL, 0, CPU_VINST, 26 }, // Bitwise Exclusive Or
    // Hardware Floating Point instructions (TODO)

    // F9445 Arithmetic Instructions (7)
    { "OR",   2, DGN_CTAA, 0b0110011100000001, CPU_F9445, 15 },
    { "NORM", 4, DGN_CT,   0b0110011011000001, CPU_F9445 | CPU_NOEMU, 0 },
    { "SLLD", 4, DGN_CT,   0b0110011000000001, CPU_F9445, 16 },
    { "SALD", 4, DGN_CT,   0b0111011000000001, CPU_F9445, 17 },
    { "SLRD", 4, DGN_CT,   0b0110011010000001, CPU_F9445, 18 },
    { "SARD", 4, DGN_CT,   0b0110011001000001, CPU_F9445, 19 },
    { "SKNV", 4, DGN_CT,   0b0110111011000001, CPU_F9445 | CPU_NOEMU, 0 },
    // F9445 Stack Instructions (7)
    { "PSHF", 4, DGN_CT,  0b0111010101000001, CPU_F9445 | CPU_NOEMU, 0 },
    { "POPF", 4, DGN_CT,  0b0111010100000001, CPU_F9445 | CPU_NOEMU, 0 },
    { "POPJ", 4, DGN_CT,  0b0111010110000001, CPU_F9445, 20 },
    { "PSHR", 4, DGN_CT,  0b0111010111000001, CPU_F9445, 21 },
    { "TOPR", 4, DGN_CTA, 0b0110001111000001, CPU_F9445, 22 }, // Conflicts with the Nova 4's PSHN
    { "TOPW", 4, DGN_CTA, 0b0110001101000001, CPU_F9445, 23 },
    { "DSP",  4, DGN_CT,  0b0111011010000001, CPU_F9445, 24 },
    // F9445 CPU Instructions (5)
    { "WAIT", 4, DGN_CT, 0b0110111000000001, CPU_F9445 | CPU_NOEMU, 0 },
    { "ETRP", 4, DGN_CT, 0b0111111001000001, CPU_F9445 | CPU_NOEMU, 0 },
    { "DTRP", 4, DGN_CT, 0b0111111011000001, CPU_F9445 | CPU_NOEMU, 0 }, // Disabling traps will break instruction ewmulation
    { "E64K", 4, DGN_CT, 0b0110111001000001, CPU_F9445 | CPU_NOEMU, 0 },
    { "D64K", 4, DGN_CT, 0b0110111011000001, CPU_F9445 | CPU_NOEMU, 0 },
    // Assembler directives (8)
    { ".TEXT", 5, ASM_TEXT, 0, CPU_BASE, 0 },
    { ".DATA", 5, ASM_DATA, 0, CPU_BASE, 0 },
    { ".BSS",  4, ASM_BSS,  0, CPU_BASE, 0 },
    { ".ZERO", 5, ASM_ZERO, 0, CPU_BASE, 0 },
    { ".GLOB", 5, ASM_GLOB, 0, CPU_BASE, 0 },
    { ".LOC",  4, ASM_LOC,  0, CPU_BASE, 0 },
    { ".ENT",  4, ASM_ENT,  0, CPU_BASE, 0 },
    { ".WSTR", 5, ASM_WSTR, 0, CPU_BASE, 0 },
    // Hardware device aliases (8 so far)
    { "MDV",  3, DGN_HWID, 001, CPU_BASE, 0 }, // Multiply & Divide Unit
    { "MAP",  3, DGN_HWID, 002, CPU_BASE, 0 }, // Memory Management Unit
    { "MAP1", 4, DGN_HWID, 003, CPU_BASE, 0 }, // Memory Management Unit (Takes up two slots)

    { "TTI", 3, DGN_HWID, 010, CPU_BASE, 0 }, // TTY Input
    { "TTO", 3, DGN_HWID, 011, CPU_BASE, 0 }, // TTY Output
    { "PTR", 3, DGN_HWID, 012, CPU_BASE, 0 }, // Paper Tape Reader
    { "PTP", 3, DGN_HWID, 013, CPU_BASE, 0 }, // Paper Tape Punch
    { "RTC", 3, DGN_HWID, 014, CPU_BASE, 0 }, // Real Time Clock
};
