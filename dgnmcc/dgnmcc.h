#include "../lib/novanix.h"
#include "../lib/a.out.h"

#define MAX_LINE 256  // Maximum number of tokens per file line
#define MAX_STR  256  // Maximum length of user defined strings
#define PAGESIZE 1024 // 2 KB (1 KW) of memory (1 mmu page)
#define ASM_SIZE 72   // Number of assembler defined symbols

// +--------------+
// | TOKEN VALUES |
// +--------------+
// Start at 128 so that we don't overlap ASCII tokens
enum
{
    Number = 128, // Numerical constant in source
    Named, // Named variable of function
    Define, // Defined pre-compiler constant
    Reserved, // Reserved word
// ******* Reserved words *******
    Void, Int, Char, Float, Long, // Types
    Auto, Signed, Const, Extern, Register, Unsigned, Static, // Storage classes
    If, Else, Case, Default, Break, Continue, Return, // Program Structure
    For, While, Do, // Loops
    Enum, Struct, Union, // Data structures
    Sizeof, // Technically this is an operator
// ******* Operators *******
    Ass, AddAss, SubAss, MulAss, DivAss, ModAss, ShlAss, ShrAss, AndAss, XorAss, OrAss, // Assignment Operators
    Tern, // Ternary Conditional
    LogOr, // Logical Or
    LogAnd, // Logical And
    Or, // Bitwise Or
    Xor, // Bitwise Xor
    And, // Bitwise And
    Eq, Ne, // Relational equal and not-equal
    Less, LessEq, Great, GreatEq, // Relational equivalencies
    Shl, Shr, // Bitshift left and right
    Add, Sub, // Addition and Subtraction
    Mul, Div, Mod, // Multiplication, Division, Modulus
    LogNot, Not, Inc, Dec, // Logical not, not, increment and decrement
    Brak, Dot, Arrow // Square bracket (array access), Dot (structure access), Arrow (structure access via pointer)
};

// Symbol type bit pattern
// |0000|0000|0|0|0|0|00|000
// |         | | | | |  |C Data type
// |         | | | | |C Storage type
// |         | | | |Signed flag
// |         | | |Extern flag
// |         | |Constant flag
// |         |Function flag

// C Data types
#define CPL_VOID 0 // Void (0 bit)
#define CPL_CHAR 1 // Character (8 bit)
#define CPL_INT  2 // Integer (16 bit)
#define CPL_LONG 3 // Long (32 bit)
#define CPL_FPV  4 // Floating point value
#define CPL_STRC 5 // Structure (Enums and Unions are technically also structs, might want to just merge them all together)
#define CPL_ENUM 6 // Enumeration
#define CPL_UNI  7 // Union

// C Storage types
#define CPL_ZERO 0 << 3 // Zero page (register)
#define CPL_TEXT 1 << 3 // Text (write only)
#define CPL_DATA 2 << 3 // Data (static)
#define CPL_STAK 3 << 3 // Stack (dynamic)

// C attribute flags
#define CPL_SIGN 1 << 5 // Signed flag
#define CPL_XTRN 1 << 6 // Extern flag
#define CPL_CNST 1 << 7 // Constant flag
#define CPL_FUNC 1 << 8 // C Function flag (text segment)

// Symbol
struct symbol
{
    char name[MAX_TOKN]; // Name of symbol

    unsigned int val; // Data stored in the symbol or address of the symbol
    unsigned int type;

    unsigned char indir; // Number of inderections
    unsigned char scope; // 0 = File scope, every { and } increase/decrease scope

    // For function symbols only
    unsigned char argc; // Number of arguments
    unsigned int dataPos; // Current offset for current member symbol on the stack
    unsigned int dataMax; // Maximum offset for all member symbols on the stack

    // Member symbols
    struct symbol * syms;
    struct symbol ** symsEnd;

    struct symbol * next; // Next node in list
};

struct relocate
{
    unsigned int head;
    unsigned int addr;
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
void mccfail(char * msg);

// Unix system calls
void exit(int status);
int creat(const char * pathname, int mode);
int  open(const char * pathname, int flags);
int close(int fd);
int  read(int fd, void * buf, unsigned int count);
int write(int fd, void * buf, unsigned int count);
void * sbrk(int inc);
