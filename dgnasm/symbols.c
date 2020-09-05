// These strings must match the symbols in the table below
char * symstrs[ASM_SIZE] = {
    "NIO", "DIA", "DOA", "DIB", "DOB", "DIC", "DOC",
    "SKPBN", "SKPBZ", "SKPDN", "SKPDZ",
    "JMP", "JSR", "ISZ", "DSZ",
    "LDA", "STA",
    "COM", "NEG", "MOV", "INC", "ADC", "SUB", "ADD", "AND",
    "TRAP",
    "LDB", "STB",
    "PSHA", "PSHN", "POPA", "SAV", "SAVN", "RET", "MTSP", "MTFP", "MFSP", "MFFP",
    "INTEN", "INTDS", "READS", "MSKO", "INTA", "IORST", "HALT",
    "MUL", "MULS", "DIV", "DIVS",
    "SKP", "SZC", "SNC", "SZR", "SNR", "SEZ", "SBN",

    // F9445 OPs
    "OR", "NORM", "SLLD", "SALD", "SLRD", "SARD", "SKNV",
    "PSHF", "POPF", "POPJ", "PSHR", "TOPR", "TOPW", "DSP",
    "WAIT", "ETRP", "DTRP", "E64K", "D64K",

    // Assembler mnemonics
    ".TEXT", ".DATA", ".BSS", ".ZERO", ".GLOB", ".DEFINE", ".ENT", ".WSTR",

    // Device mnemonics
    "MDV", "MAP", "MAP1", "TTI", "TTO", "PTR", "PTP", "RTC"
};

struct asmsym ** symtail, ** symtbl, symint[ASM_SIZE] = {
    // I/O Instructions (7)
    { NULL, DGN_IONO, 3, 0b0110000000000000, symint +  1 }, // NIO
    { NULL, DGN_IO,   3, 0b0110000100000000, symint +  2 }, // DIA
    { NULL, DGN_IO,   3, 0b0110001000000000, symint +  3 }, // DOA
    { NULL, DGN_IO,   3, 0b0110001100000000, symint +  4 }, // DIB
    { NULL, DGN_IO,   3, 0b0110010000000000, symint +  5 }, // DOB
    { NULL, DGN_IO,   3, 0b0110010100000000, symint +  6 }, // DIC
    { NULL, DGN_IO,   3, 0b0110011000000000, symint +  7 }, // DOC
    // I/O Skip instructions (4)
    { NULL, DGN_IOSK, 5, 0b0110011100000000, symint +  8 }, // SKPBN
    { NULL, DGN_IOSK, 5, 0b0110011101000000, symint +  9 }, // SKPBZ
    { NULL, DGN_IOSK, 5, 0b0110011110000000, symint + 10 }, // SKPDN
    { NULL, DGN_IOSK, 5, 0b0110011111000000, symint + 11 }, // SKPDZ
    // Flow control instructions (4)
    { NULL, DGN_FLOW, 3, 0b0000000000000000, symint + 12 }, // JMP
    { NULL, DGN_FLOW, 3, 0b0000100000000000, symint + 13 }, // JSR
    { NULL, DGN_FLOW, 3, 0b0001000000000000, symint + 14 }, // ISZ
    { NULL, DGN_FLOW, 3, 0b0001100000000000, symint + 15 }, // DSZ
    // Memory access instructions (2)
    { NULL, DGN_LOAD, 3, 0b0010000000000000, symint + 16 }, // LDA
    { NULL, DGN_LOAD, 3, 0b0100000000000000, symint + 17 }, // STA
    // Arithmetic & Logic instructions (8)
    { NULL, DGN_MATH, 3, 0b1000000000000000, symint + 18 }, // COM
    { NULL, DGN_MATH, 3, 0b1000000100000000, symint + 19 }, // NEG
    { NULL, DGN_MATH, 3, 0b1000001000000000, symint + 20 }, // MOV
    { NULL, DGN_MATH, 3, 0b1000001100000000, symint + 21 }, // INC
    { NULL, DGN_MATH, 3, 0b1000010000000000, symint + 22 }, // ADC
    { NULL, DGN_MATH, 3, 0b1000010100000000, symint + 23 }, // SUB
    { NULL, DGN_MATH, 3, 0b1000011000000000, symint + 24 }, // ADD
    { NULL, DGN_MATH, 3, 0b1000011100000000, symint + 25 }, // AND
    // Trap instruction (Arithmetic no-op) (1)
    { NULL, DGN_TRAP, 4, 0b1000000000001000, symint + 26 }, // TRAP
    // Byte acess instructions (2)
    { NULL, DGN_CTAA, 3, 0b0110000100000001, symint + 27 }, // LDB
    { NULL, DGN_CTAA, 3, 0b0110010000000001, symint + 28 }, // STB
    // Stack instructions (10)
    { NULL, DGN_CTA,  4, 0b0110001100000001, symint + 29 }, // PSHA
    { NULL, DGN_CTA,  4, 0b0110001111000001, symint + 30 }, // PSHN - [Undocumented] PSHA With I/O Pulse
    { NULL, DGN_CTA,  4, 0b0110001110000001, symint + 31 }, // POPA
    { NULL, DGN_CT,   3, 0b0110010100000001, symint + 32 }, // SAV
    { NULL, DGN_CT,   4, 0b0110010111000001, symint + 33 }, // SAVN - [Undocumented] SAV with I/O Pulse
    { NULL, DGN_CT,   3, 0b0110010110000001, symint + 34 }, // RET
    { NULL, DGN_CTA,  4, 0b0110001000000001, symint + 35 }, // MTSP
    { NULL, DGN_CTA,  4, 0b0110000000000001, symint + 36 }, // MTFP
    { NULL, DGN_CTA,  4, 0b0110001010000001, symint + 37 }, // MFSP
    { NULL, DGN_CTA,  4, 0b0110000010000001, symint + 38 }, // MFFP
    // CPU Control instructions (7)
    { NULL, DGN_CT,   5, 0b0110000001111111, symint + 39 }, // INTEN
    { NULL, DGN_CT,   5, 0b0110000010111111, symint + 40 }, // INTDS
    { NULL, DGN_CTAF, 5, 0b0110000100111111, symint + 41 }, // READS
    { NULL, DGN_CTAF, 4, 0b0110010000111111, symint + 42 }, // MSKO
    { NULL, DGN_CTAF, 4, 0b0110001100111111, symint + 43 }, // INTA
    { NULL, DGN_CTF,  5, 0b0110010110111111, symint + 44 }, // IORST
    { NULL, DGN_CTF,  4, 0b0110011000111111, symint + 45 }, // HALT
    // Hardware Multiply & Divide instructions (4)
    { NULL, DGN_CT,   3, 0b0111011011000001, symint + 46 }, // MUL
    { NULL, DGN_CT,   4, 0b0111111010000001, symint + 47 }, // MULS
    { NULL, DGN_CT,   3, 0b0111011001000001, symint + 48 }, // DIV
    { NULL, DGN_CT,   4, 0b0111111000000001, symint + 49 }, // DIVS
    // Hardware Floating Point instructions (TODO)

    // Arithmetic & Logic skip conditions (7)
    { NULL, DGN_SKPC, 3, 01, symint + 50 }, // SKP
    { NULL, DGN_SKPC, 3, 02, symint + 51 }, // SZC
    { NULL, DGN_SKPC, 3, 03, symint + 52 }, // SNC
    { NULL, DGN_SKPC, 3, 04, symint + 53 }, // SZR
    { NULL, DGN_SKPC, 3, 05, symint + 54 }, // SNR
    { NULL, DGN_SKPC, 3, 06, symint + 55 }, // SEZ
    { NULL, DGN_SKPC, 3, 07, symint + 56 }, // SBN
    // F9445 Arithmetic Instructions (7)
    { NULL, DGN_CTAA, 2, 0b0110011100000001, symint + 57 }, // OR
    { NULL, DGN_CT,   4, 0b0110011011000001, symint + 58 }, // NORM
    { NULL, DGN_CT,   4, 0b0110011000000001, symint + 59 }, // SLLD
    { NULL, DGN_CT,   4, 0b0111011000000001, symint + 60 }, // SALD
    { NULL, DGN_CT,   4, 0b0110011010000001, symint + 61 }, // SLRD
    { NULL, DGN_CT,   4, 0b0110011001000001, symint + 62 }, // SARD
    { NULL, DGN_CT,   4, 0b0110111011000001, symint + 63 }, // SKNV
    // F9445 Stack Instructions (7)
    { NULL, DGN_CT,   4, 0b0111010101000001, symint + 64 }, // PSHF
    { NULL, DGN_CT,   4, 0b0111010100000001, symint + 65 }, // POPF
    { NULL, DGN_CT,   4, 0b0111010110000001, symint + 66 }, // POPJ
    { NULL, DGN_CT,   4, 0b0111010111000001, symint + 67 }, // PSHR
    { NULL, DGN_CTA,  4, 0b0110001111000001, symint + 68 }, // TOPR
    { NULL, DGN_CTA,  4, 0b0110001101000001, symint + 69 }, // TOPW
    { NULL, DGN_CT,   3, 0b0111011010000001, symint + 70 }, // DSP
    // F9445 CPU Instructions (5)
    { NULL, DGN_CT,   4, 0b0110111000000001, symint + 71 }, // WAIT
    { NULL, DGN_CT,   4, 0b0111111001000001, symint + 72 }, // ETRP
    { NULL, DGN_CT,   4, 0b0111111011000001, symint + 73 }, // DTRP
    { NULL, DGN_CT,   4, 0b0110111001000001, symint + 74 }, // E64K
    { NULL, DGN_CT,   4, 0b0110111011000001, symint + 75 }, // D64K
    // Assembler directives (6)
    { NULL, ASM_TEXT, 5, 0, symint + 76 }, // .TEXT
    { NULL, ASM_DATA, 5, 0, symint + 77 }, // .DATA
    { NULL, ASM_BSS,  4, 0, symint + 78 }, // .BSS
    { NULL, ASM_ZERO, 5, 0, symint + 79 }, // .ZERO
    { NULL, ASM_GLOB, 5, 0, symint + 80 }, // .GLOB
    { NULL, ASM_DEFN, 7, 0, symint + 81 }, // .DEFINE
    { NULL, ASM_ENT,  4, 0, symint + 82 }, // .ENT
    { NULL, ASM_WSTR, 5, 0, symint + 83 }, // .WSTR (Word String)
    // Hardware device aliases (8 so far)
    { NULL, DGN_HWID, 3, 001, symint + 84 }, // MDV - Multiply & Divide
    { NULL, DGN_HWID, 3, 002, symint + 85 }, // MAP  - Memory Management Unit
    { NULL, DGN_HWID, 4, 003, symint + 86 }, // MAP1 - Memory Management Unit (Takes up two slots)

    { NULL, DGN_HWID, 3, 010, symint + 87 }, // TTI - TTY input
    { NULL, DGN_HWID, 3, 011, symint + 88 }, // TTO - TTY output
    { NULL, DGN_HWID, 3, 012, symint + 89 }, // PTR - Paper tape reader
    { NULL, DGN_HWID, 3, 013, symint + 90 }, // PTP - Paper tape punch
    { NULL, DGN_HWID, 3, 014, NULL }, // RTC - Real time clock
};
