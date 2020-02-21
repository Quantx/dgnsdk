// Pretend this is a 16-bit system
#define int short
#define long int

// How many symbols should we allocate for
#define MAX_SYMS 512

//   #######   Symbol Types   #######
// *** Instruction parameter formats ***
#define SYM_IONO  0 // I/O No accumulator
#define SYM_IO    1 // I/O
#define SYM_IOSK  2 // I/O Skip
#define SYM_FLOW  3 // Flow control
#define SYM_LOAD  4 // Memory access
#define SYM_MATH  5 // Arithmetic, Logic
// *** Extened Instruction parameter formats ***
#define SYM_TRAP  6 // Arithmetic no-op (No load, no skip)
#define SYM_CT    7 // Control, no accumulator or flags
#define SYM_CTA   8 // Control, acumulator
#define SYM_CTF   9 // Control, flag
#define SYM_CTAF 10 // Control, acumulator and flag
#define SYM_CTAA 11 // Control, two accumulators
// *** Assembler constants ***
#define SYM_SKPC 12 // Arithmetic & Logic skip condition
#define SYM_HWID 13 // Nova Hardware ID
// *** User symbols ***
#define SYM_DEF  14 // Pre-processor define
#define SYM_TXT  15 // Text pointer
#define SYM_DAT  16 // Data pointer
#define SYM_BSS  17 // Bss  pointer
#define SYM_GTXT 18 // Global text pointer
#define SYM_GDAT 19 // Global data pointer
#define SYM_GBSS 20 // Global bss  pointer

struct symbol
{
    // The Unix6 'a.out' executable standard limits us to 8 characters
    char name[8];
    int type;
    int val; // Opcode, Address, Value, etc.
};
