#include "../lib/novanix.h"
#include "../lib/a.out.h"

#define MAX_INCS 8    // Maximum depth of includes
#define MAX_LINE 256  // Maximum number of tokens per file line
#define MAX_STR  256  // Maximum length of user defined strings
#define PAGESIZE 1024 // 2 KB (1 KW) of memory (1 mmu page)
#define MAX_TOKN 32   // Maximum length of a symbol name

// +--------------+
// | TOKEN VALUES |
// +--------------+
// Start at 128 so that we don't overlap ASCII tokens
enum
{
    Number = 128, // Numerical constant in source
    Named, // Named variable of function
// ******* Reserved words *******
    Void, Int, Short, Char, Float, Long, // Types
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
    Eq, Neq, // Relational equal and not-equal
    Less, LessEq, Great, GreatEq, // Relational equivalencies
    Shl, Shr, // Bitshift left and right
    Add, Sub, // Addition and Subtraction
    Mul, Div, Mod, // Multiplication, Division, Modulus
    LogNot, Not, Inc, Dec, // Logical not, not, increment and decrement
    Brak, Dot, Arrow // Square bracket (array access), Dot (structure access), Arrow (structure access via pointer)
};

// Symbol type bit pattern
// |0000|0000|0|0|0|00|000|0
// |         | | | |  |   |Defined flag
// |         | | | |  |C Data type
// |         | | | |C Storage type
// |         | | |Signed flag
// |         | |Extern flag
// |         |Function flag

// Defined flag (set once defined)
#define CPL_DEFN 1

// C Data types
#define CPL_VOID 0 << 1 // Void (0 bit)
#define CPL_CHAR 1 << 1 // Character (8 bit)
#define CPL_INT  2 << 1 // Integer (16 bit)
#define CPL_LONG 3 << 1 // Long (32 bit)
#define CPL_FPV  4 << 1 // Floating point value
#define CPL_STRC 5 << 1 // Structure (Enums and Unions are technically also structs, might want to just merge them all together)
#define CPL_ENUM 6 << 1 // Enumeration
#define CPL_UNI  7 << 1 // Union
#define CPL_TYPE_MASK 7 << 1 // Mask

// C Storage types
#define CPL_TEXT 0 << 4 // Text (write only) (functions and constants)
#define CPL_ZERO 1 << 4 // Zero page (register)
#define CPL_DATA 2 << 4 // Data (static)
#define CPL_STAK 3 << 4 // Stack (dynamic)
#define CPL_STORE_MASK 3 << 4 // Mask

// C attribute flags
#define CPL_SIGN 1 << 6 // Signed flag
#define CPL_XTRN 1 << 7 // Extern flag
#define CPL_FUNC 1 << 8 // Function flag

// Symbol
struct mccsym
{
    unsigned int val; // Data stored in the symbol or address of the symbol

    char name[MAX_TOKN]; // Name of symbol
    unsigned char len; // Length of symbol in bytes
    unsigned char type;

    unsigned char indir; // Number of inderections
    unsigned char scope; // 0 = File scope, every { and } increase/decrease scope
/*
    // For function symbols only
    unsigned char argc; // Number of arguments
    unsigned int dataPos; // Current offset for current member symbol on the stack
    unsigned int dataMax; // Maximum offset for all member symbols on the stack

    // Member symbols (function arguments)
    struct symbol * syms, ** symsEnd, * symsNext;

    struct symbol * next, * prev;
*/
};

// Store a data segment
struct segment
{
    // Segment's data as it appears in memory
    struct
    {
        int fd;
        unsigned int pos;
        unsigned int size;
    } data;

    // Relocation information for this segment
    struct
    {
        int fd;
        unsigned int pos;
        unsigned int size;
    } rloc;
};

// Assembly fail function
void mccfail(char * msg);
