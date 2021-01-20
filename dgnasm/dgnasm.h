#include "../lib/novanix.h"
#include "../lib/a.out.h"

//#define DBUG_TOK 1
//#define DBUG_SYM 1

#define MAX_LINE  256  // Maximum number of tokens per file line
#define MAX_STR   256  // Maximum length of user defined strings
#define PAGESIZE  1024 // 2 KB (1 KW) of memory (1 mmu page)
#define CBUF_LEN  256  // Size of buffer used to copy from one file to another (Must be at least 16)

// *** DGNova Instruction Parameter Formats ***
enum {
    DGN_NULL = 16, // Unsupported instruction
    DGN_IONO, // I/O No accumulator
    DGN_IO,   // I/O
    DGN_IOSK, // I/O Skip
    DGN_FLOW, // Flow control
    DGN_LOAD, // Memory access
    DGN_MATH, // Arithmetic, Logic
// *** DGNova Extened Instruction Parameter Formats ***
    DGN_TRAP, // Trap instruction (Arithmetic no-op (No load, no skip))
    DGN_CT,   // Control, no accumulator or flags
    DGN_CTF,  // Control, flag
    DGN_CTA,  // Control, acumulator
    DGN_CTAF, // Control, acumulator and flag
    DGN_CTAA, // Control, two accumulators
    DGN_VI,   // Virtual Instruction
    DGN_VIA,  // Virtual Instruction, accumulator
    DGN_VIAA, // Virtual Instruction, two accumulators
// *** Assembler directive ***
    ASM_TEXT, // .text directive
    ASM_DATA, // .data directive
    ASM_BSS,  // .bss  directive
    ASM_ZERO, // .zero directive
    ASM_GLOB, // .glob directive
    ASM_LOC,  // .loc  directive
    ASM_ENT,  // .ent  directive
    ASM_WSTR, // .wstr directive
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
#define FLG_GLOB   0b0000000000000001 // Force all undefined symbols to be global
#define FLG_DATA   0b0000000000000010 // Are we on the second pass?
#define FLG_SMH    0b0000000000000100 // Use SimH output
#define FLG_SMHA   0b0000000000001100 // Use SimH output with auto start
#define FLG_TERM   0b0000000000001000 // Use the Nova 4 virtual console output
#define FLG_VIEMU  0b0000000000010000 // Virtual Instruction Emulation

// Which platforms is an instruction native to?
#define CPU_VINST  0b00000000 // Virtual Instruction
#define CPU_BASE   0b00011111 // Base Nova instruction (present on all units)
#define CPU_ETRAP  0b00000011 // Systems which need emulated trap instructions
#define CPU_NOVA1  0b00000001 // Nova
#define CPU_MDV    0b00000010 // Nova with MDV unit (present on all subsequent units)
#define CPU_NOVA3  0b00000100 // Nova 3
#define CPU_NOVA4  0b00001000 // Nova 4
#define CPU_F9445  0b00010000 // Fairchild F9445
#define CPU_NOEMU  0b10000000 // Do not emulate flag

struct instruct
{
    int8_t * name;
    unsigned int8_t len;

    unsigned int8_t type;
    unsigned int16_t val;

    unsigned int8_t target;
    unsigned int8_t trap;
};

struct asmsym
{
    int8_t * name;
    unsigned int8_t len; // Length of name in bytes

    unsigned int8_t type; // Type of symbol
    unsigned int16_t val;

    struct asmsym * next;
};

// Used to substitute instructions with hardware traps
struct asmtrap
{
    unsigned int8_t index; // Index of the instruction we're replacing
    unsigned int8_t type; // Type of trap & compile target
    unsigned int16_t trap; // Trap code we're replacing it with
};

// Store a data segment
struct segment
{
    struct
    {
       int16_t fd;
       unsigned int16_t pos;
       unsigned int16_t size;
    } data, // Core image
      rloc; // Relocation info

    unsigned int8_t sym; // The symbol type
};

// Assembly fail function
void asmfail(int8_t * msg);
