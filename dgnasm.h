#include <stdlib.h>
#include <unistd.h>

// Pretend this is a 16-bit system
#define int short
#define long int

// How many symbols should we allocate for
#define MAX_SYMS 512
#define MAX_LINE 256
#define MAX_TOKN 8

//   #######   Symbol Types   #######
// *** Instruction parameter formats ***
#define SYM_IONO 128 // I/O No accumulator
#define SYM_IO   129 // I/O
#define SYM_IOSK 130 // I/O Skip
#define SYM_FLOW 131 // Flow control
#define SYM_LOAD 132 // Memory access
#define SYM_MATH 133 // Arithmetic, Logic
// *** Extened Instruction parameter formats ***
#define SYM_TRAP 134 // Arithmetic no-op (No load, no skip)
#define SYM_CT   135 // Control, no accumulator or flags
#define SYM_CTA  136 // Control, acumulator
#define SYM_CTF  137 // Control, flag
#define SYM_CTAF 138 // Control, acumulator and flag
#define SYM_CTAA 139 // Control, two accumulators
// *** Assembler constants ***
#define SYM_SKPC 140 // Arithmetic & Logic skip condition
#define SYM_HWID 141 // Nova Hardware ID
// *** User symbols ***
#define SYM_DEF  142 // Pre-processor define
#define SYM_TXT  143 // Text pointer
#define SYM_DAT  144 // Data pointer
#define SYM_BSS  145 // Bss  pointer
#define SYM_GTXT 146 // Global text pointer
#define SYM_GDAT 147 // Global data pointer
#define SYM_GBSS 148 // Global bss  pointer
// *** Other tokens ***
#define TOK_NAME 149 // Named symbol
#define TOK_NUM  150 // Numberical constant
#define TOK_SEP  151 // Seperator token (comma)
#define TOK_ADD  152 // Addition
#define TOK_SUB  153 // Subtraction
#define TOK_MUL  154 // Multiply
#define TOK_DIV  155 // Divide
#define TOK_MOD  156 // Modulous

// *** Symbol flags ***
#define SYM_EXST 0x8000 // Flag set when symbol is defined

struct symbol
{
    // The Unix6 'a.out' executable standard limits us to 8 characters
    char name[8];
    unsigned int type;
    unsigned int val; // Opcode, Address, Value, etc.
};
