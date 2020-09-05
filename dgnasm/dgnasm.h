#include "../lib/novanix.h"
#include "../lib/a.out.h"

//#define DBUG_TOK 1
//#define DBUG_SYM 1

#define MAX_LINE 256  // Maximum number of tokens per file line
#define MAX_STR  256  // Maximum length of user defined strings
#define PAGESIZE 1024 // 2 KB (1 KW) of memory (1 mmu page)
#define ASM_SIZE 91   // Number of assembler defined symbols
#define CBUF_LEN 256  // Size of buffer used to copy from one file to another (Must be at least 16)

// *** DGNova Instruction Parameter Formats ***
enum {
    DGN_IONO = 16, // I/O No accumulator
    DGN_IO,   // I/O
    DGN_IOSK, // I/O Skip
    DGN_FLOW, // Flow control
    DGN_LOAD, // Memory access
    DGN_MATH, // Arithmetic, Logic
// *** DGNova Extened Instruction Parameter Formats ***
    DGN_TRAP, // Arithmetic no-op (No load, no skip)
    DGN_CT,   // Control, no accumulator or flags
    DGN_CTA,  // Control, acumulator
    DGN_CTF,  // Control, flag
    DGN_CTAF, // Control, acumulator and flag
    DGN_CTAA, // Control, two accumulators
// *** Assembler directive ***
    ASM_TEXT, // .text   directive
    ASM_DATA, // .data   directive
    ASM_BSS,  // .bss    directive
    ASM_ZERO, // .zero   directive
    ASM_GLOB, // .glob   directive
    ASM_DEFN, // .define directive
    ASM_ENT,  // .ent    directive
    ASM_WSTR, // .wstr   directive
// *** DGNova Constants ***
    DGN_SKPC, // Arithmetic & Logic skip condition
    DGN_HWID, // Nova Hardware ID
// *** Token types ***
    TOK_NAME, // Named symbol (user label)
    TOK_NUM,  // Numberical constant
    TOK_INDR, // Indirect bit token '@'
    TOK_LABL, // Label token ':'
    TOK_ARG,  // Argument seperator token ','
    TOK_BYLO, // Low  byte indicator '>'
    TOK_BYHI, // High byte indicator '<'
    TOK_EOL,  // End of line token
    TOK_MATH, // Plus or minus
    TOK_STR  // User defined string
};

// *** Boolean Flags ***
#define FLG_GLOB 0b0000000000000001 // Force all undefined symbols to be global
#define FLG_DATA 0b0000000000000010 // Are we on the second pass?
#define FLG_SMH  0b0000000000000100 // Use SimH output
#define FLG_SMHA 0b0000000000001100 // Use SimH output with auto start
#define FLG_TERM 0b0000000000001000 // Use the Nova 4 virtual console output

struct asmsym
{
    char * name;
    unsigned char type; // Type of symbol
    unsigned char len; // Length of name in bytes
    unsigned int val;

    struct asmsym * next;
};

// Store a data segment
struct segment
{
    struct
    {
       int fd;
       unsigned int pos;
       unsigned int size;
    } data, // Core image
      rloc; // Relocation info

    unsigned char sym; // The symbol type
};

// Assembly fail function
void asmfail(char * msg);
