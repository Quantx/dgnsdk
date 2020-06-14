#include "../lib/novanix.h"
#include "../lib/a.out.h"

//#define DBUG_TOK 1
//#define DBUG_SYM 1

#define MAX_LINE 256  // Maximum number of tokens per file line
#define MAX_STR  256  // Maximum length of user defined strings
#define PAGESIZE 1024 // 2 KB (1 KW) of memory (1 mmu page)
#define ASM_SIZE 72   // Number of assembler defined symbols
#define MAX_TOKN 32   // Maximum number of characters in a single symbol

// *** DGNova Instruction Parameter Formats ***
#define DGN_IONO 16 // I/O No accumulator
#define DGN_IO   17 // I/O
#define DGN_IOSK 18 // I/O Skip
#define DGN_FLOW 19 // Flow control
#define DGN_LOAD 20 // Memory access
#define DGN_MATH 21 // Arithmetic, Logic
// *** DGNova Extened Instruction Parameter Formats ***
#define DGN_TRAP 22 // Arithmetic no-op (No load, no skip)
#define DGN_CT   23 // Control, no accumulator or flags
#define DGN_CTA  24 // Control, acumulator
#define DGN_CTF  25 // Control, flag
#define DGN_CTAF 26 // Control, acumulator and flag
#define DGN_CTAA 27 // Control, two accumulators
// *** Assembler directive ***
#define ASM_TEXT 28 // .text   directive
#define ASM_DATA 29 // .data   directive
#define ASM_BSS  30 // .bss    directive
#define ASM_ZERO 31 // .zero   directive
#define ASM_GLOB 32 // .glob   directive
#define ASM_DEFN 33 // .define directive
#define ASM_ENT  34 // .ent    directive
#define ASM_WSTR 35 // .wstr   directive
// *** DGNova Constants ***
#define DGN_SKPC 36 // Arithmetic & Logic skip condition
#define DGN_HWID 37 // Nova Hardware ID
// *** Token types ***
#define TOK_NAME 38 // Named symbol (user label)
#define TOK_NUM  39 // Numberical constant
#define TOK_INDR 40 // Indirect bit token '@'
#define TOK_LABL 41 // Label token ':'
#define TOK_ARG  42 // Argument seperator token ','
#define TOK_BYLO 43 // Low  byte indicator '>'
#define TOK_BYHI 44 // High byte indicator '<'
#define TOK_EOL  45 // End of line token
#define TOK_MATH 46 // Plus or minus
#define TOK_STR  47 // User defined string

// *** Boolean Flags ***
#define FLG_GLOB 0b0000000000000001 // Force all undefined symbols to be global
#define FLG_RLOC 0b0000000000000010 // Should we record relocation bits?
#define FLG_DATA 0b0000000000000100 // Should we write out data this pass?
#define FLG_SMH  0b0000000000001000 // Use SimH output
#define FLG_SMHA 0b0000000000011000 // Use SimH output with auto start
#define FLG_TERM 0b0000000000010000 // Use the Nova 4 virtual console output
#define FLG_LOCL 0b0000000000100000 // Output local symbols

struct asmsym
{
    char name[MAX_TOKN]; // Name of symbol
    unsigned char type; // Type of symbol
    unsigned char len; // Length of name in bytes
    unsigned int val;
};

// Store a data segment
struct segment
{
    // Segment's data as it appears in memory
    unsigned int * data;
    unsigned int dataPos;
    unsigned int dataSize;

    // Relocation information for this segment
    struct relocate * rloc;
    unsigned int rlocPos;
    unsigned int rlocSize;

    unsigned char sym; // The symbol type
};

// Assembly fail function
void asmfail(char * msg);
