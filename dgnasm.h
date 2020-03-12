// Pretend this is a 16-bit unix 6 system
#define int short
#define NULL 0

#define DBUG_TOK 1
#define DBUG_SYM 1

// How many symbols should we allocate for
#define MAX_SYMS 1024
#define MAX_LINE 256
#define MAX_TOKN 8    // The Unix6 'a.out' executable standard limits us to 8 characters
#define PAGESIZE 1023 // 2 KB (1 KW) of memory (1 mmu page)

// *** DGNova Instruction Parameter Formats ***
#define DGN_IONO  0 // I/O No accumulator
#define DGN_IO    1 // I/O
#define DGN_IOSK  2 // I/O Skip
#define DGN_FLOW  3 // Flow control
#define DGN_LOAD  4 // Memory access
#define DGN_MATH  5 // Arithmetic, Logic
// *** DGNova Extened Instruction Parameter Formats ***
#define DGN_TRAP  6 // Arithmetic no-op (No load, no skip)
#define DGN_CT    7 // Control, no accumulator or flags
#define DGN_CTA   8 // Control, acumulator
#define DGN_CTF   9 // Control, flag
#define DGN_CTAF 10 // Control, acumulator and flag
#define DGN_CTAA 11 // Control, two accumulators
// *** Assembler directive ***
#define ASM_TEXT 12 // .text   directive
#define ASM_DATA 13 // .data   directive
#define ASM_BSS  14 // .bss    directive
#define ASM_ZERO 15 // .zero   directive
#define ASM_GLOB 16 // .glob   directive
#define ASM_DEFN 17 // .define directive
// *** DGNova Constants ***
#define DGN_SKPC 18 // Arithmetic & Logic skip condition
#define DGN_HWID 19 // Nova Hardware ID
// *** Token types ***
#define TOK_NAME 20 // Named symbol (user label)
#define TOK_NUM  21 // Numberical constant
#define TOK_INDR 22 // Indirect bit token '@'
#define TOK_LABL 23 // Label token ':'
#define TOK_ARG  24 // Argument seperator token ','
#define TOK_BYLO 25 // Low  byte indicator '>'
#define TOK_BYHI 26 // High byte indicator '<'
#define TOK_EOL  27 // End of line token

// *** Symbols Type ***
#define SYM_BYTE  1 // Byte flag

#define SYM_ABS   0 // Constant value
#define SYM_TEXT  2 // Text pointer
#define SYM_DATA  4 // Data pointer
#define SYM_BSS   6 // Bss  pointer
#define SYM_ZERO  8 // Zero-page pointer
#define SYM_DEF  10 // Undefined symbol
#define SYM_ZDSP 12 // Zero-page mode 0 displacement
#define SYM_ZDEF 14 // Undefined zero page symbol
#define SYM_FILE 16 // File seperator symbol

// *** Boolean Flags ***
#define FLG_GLOB 0b0000000000000001 // Force all undefined symbols to be global

struct symbol
{
    char name[MAX_TOKN]; // Symbol name
    unsigned char type; // Symbol type
    unsigned char file; // What file this symbol is local to (0 = global)
    unsigned int val; // Opcode, Address, Value, etc.
};

// Block of memory, used to hold newly defined blocks
struct memblock
{
    unsigned int data[PAGESIZE]; // Data
    struct memblock * next; // Next memblock in list
};

// Store a data segment
struct segment
{
    unsigned int pos; // Current position in the segment
    unsigned int max; // Highest position reached in the segment

    struct memblock * head; // Actuall data stored in segment
    struct memblock * rdir; // Redirection bits for the data

    unsigned char sym; // The symbol type
};

// Assembly fail function
void asmfail(char * msg);

// Unix system calls
void exit(int status);
int  open(const char * pathname, int flags);
int close(int fd);
int  read(int fd, void * buf, unsigned int count);
int write(int fd, void * buf, unsigned int count);
void * sbrk(int inc);
