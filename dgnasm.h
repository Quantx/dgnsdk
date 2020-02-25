// Pretend this is a 16-bit system
#define int short
#define long int

// How many symbols should we allocate for
#define MAX_SYMS 512
#define MAX_LINE 256
#define MAX_TOKN 8

// *** Instruction Parameter Formats ***
#define TOK_IONO 128 // I/O No accumulator
#define TOK_IO   129 // I/O
#define TOK_IOSK 130 // I/O Skip
#define TOK_FLOW 131 // Flow control
#define TOK_LOAD 132 // Memory access
#define TOK_MATH 133 // Arithmetic, Logic
// *** Extened Instruction Parameter Formats ***
#define TOK_TRAP 134 // Arithmetic no-op (No load, no skip)
#define TOK_CT   135 // Control, no accumulator or flags
#define TOK_CTA  136 // Control, acumulator
#define TOK_CTF  137 // Control, flag
#define TOK_CTAF 138 // Control, acumulator and flag
#define TOK_CTAA 139 // Control, two accumulators
// *** Assembler Constants ***
#define TOK_SKPC 140 // Arithmetic & Logic skip condition
#define TOK_HWID 141 // Nova Hardware ID
// *** User Constants ***
#define TOK_NAME 142 // Named symbol (user label)
#define TOK_NUM  143 // Numberical constant
// *** User Symbols ***
#define SYM_CNST 144 // Pre-processor define constant
#define SYM_DEF  145 // Undefined symbol
#define SYM_TXT  146 // Text pointer
#define SYM_DAT  147 // Data pointer
#define SYM_BSS  148 // Bss  pointer
#define SYM_GDEF 149 // Global undefined symbol
#define SYM_GTXT 150 // Global text pointer
#define SYM_GDAT 151 // Global data pointer
#define SYM_GBSS 152 // Global bss  pointer

struct symbol
{
    // The Unix6 'a.out' executable standard limits us to 8 characters
    char name[8];
    unsigned int type;
    unsigned int val; // Opcode, Address, Value, etc.
};

// Unix system calls
void exit(int status);
int  open(const char * pathname, int flags);
int close(int fd);
int  read(int fd, void * buf, unsigned int count);
int write(int fd, void * buf, unsigned int count);
