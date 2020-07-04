// a.out file format
// Header (struct exec)
// ---------------------------------
// Zero segment image
// Text segment image
// Data segment image
// ---------------------------------
// Symbol table (struct symbol)
// String table (null terminated)
// ---------------------------------
// Zero relocation info
// Text relocation info
// Data relocation info


// Relocation bit pattern
// |000000000000|000|0
// |            |   |Byte flag
// |            |Symbol table type
// |Symbol number (4096 possible values)

// Symbol type pattern
// |000000000000|000|0
// |            |   |Global flag
// |            |Symbol type

// *** Symbols Type ***
#define SYM_BYTE  1 // Byte flag

#define SYM_ABS   0 << 1 // Constant value
#define SYM_ZERO  1 << 1 // Zero-page pointer
#define SYM_TEXT  2 << 1 // Text pointer
#define SYM_DATA  3 << 1 // Data pointer
#define SYM_BSS   4 << 1 // Bss  pointer
#define SYM_DEF   5 << 1 // Undefined symbol
#define SYM_ZDEF  6 << 1 // Undefined zero page symbol
#define SYM_FILE  7 << 1 // File seperator symbol

#define SYM_MASK  7 << 1 // Symbol type mask

#define SYM_GLOB  8 // Global flag

struct symbol
{
    union {
        char * name; // Name of the symbol
        unsigned int stroff; // Offset of the start of the string
    };

    unsigned int type; // Symbol type
    unsigned int val; // Opcode, Address, Value, etc.
};

struct relocate
{
    unsigned int head;
    unsigned int addr;
};

struct exec
{
    unsigned char magic;  // Magic number (program load info)
    unsigned char stack;  // Size of stack in 2KB (1KW) intervals (0 to 31)
    unsigned char zero;   // Size of zero page segment in words
    unsigned char zrsize; // Number of reloaction enteries for the zero page segment
    unsigned int  text;   // Size of text segment in words
    unsigned int  trsize; // Number of relocation enteries for the text segment
    unsigned int  data;   // Size of data segment in words
    unsigned int  drsize; // Number of relocation enteries for the data segment
    unsigned int  bss;    // Size of bss  segment in words
    unsigned int  syms;   // Number of symbols in table
    unsigned int  entry;  // Address of first instruction to execute
};
