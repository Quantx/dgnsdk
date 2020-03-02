// Pretend this is a 16-bit unix 6 system
#define int short
#define long int
#define NULL 0

// How many symbols should we allocate for
#define MAX_SYMS 512
#define MAX_LINE 256
#define MAX_TOKN 8
#define PAGESIZE 1023 // 2 KB (1 KW) of memory (1 mmu page)

// *** Instruction Parameter Formats ***
#define OPC_IONO 128 // I/O No accumulator
#define OPC_IO   129 // I/O
#define OPC_IOSK 130 // I/O Skip
#define OPC_FLOW 131 // Flow control
#define OPC_LOAD 132 // Memory access
#define OPC_MATH 133 // Arithmetic, Logic
// *** Extened Instruction Parameter Formats ***
#define OPC_TRAP 134 // Arithmetic no-op (No load, no skip)
#define OPC_CT   135 // Control, no accumulator or flags
#define OPC_CTA  136 // Control, acumulator
#define OPC_CTF  137 // Control, flag
#define OPC_CTAF 138 // Control, acumulator and flag
#define OPC_CTAA 139 // Control, two accumulators
// *** Assembler Constants ***
#define TOK_SKPC 140 // Arithmetic & Logic skip condition
#define TOK_HWID 141 // Nova Hardware ID
// *** User Constans ***
#define TOK_NAME 142 // Named symbol (user label)
#define TOK_NUM  143 // Numberical constant
#define TOK_MATH 144 // Math symbol
// *** User Symbols ***
#define SYM_DEF  145 // Undefined symbol
#define SYM_ABS  146 // Absolute symbol
#define SYM_TXT  147 // Text pointer
#define SYM_DAT  148 // Data pointer
#define SYM_BSS  159 // Bss  pointer
#define SYM_GDEF 150 // Global undefined symbol
#define SYM_GABS 151 // Global absolute symbol
#define SYM_GTXT 152 // Global text pointer
#define SYM_GDAT 153 // Global data pointer
#define SYM_GBSS 154 // Global bss  pointer

// *** Boolean Flags ***
#define FLG_GLOB 0b0000000000000001 // Force all undefined symbols to be global

struct symbol
{
    // The Unix6 'a.out' executable standard limits us to 8 characters
    char name[8];
    unsigned char type; // Symbol type
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
    struct memblock * head; // Actuall data stored in segment
    struct memblock * rdir; // Redirection bits for the data

    unsigned char lsym; // The local symbol type
    unsigned char gsym; // The global symbol type
};

// Unix system calls
void exit(int status);
int  open(const char * pathname, int flags);
int close(int fd);
int  read(int fd, void * buf, unsigned int count);
int write(int fd, void * buf, unsigned int count);
void * sbrk(int inc);
