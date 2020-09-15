#include "../lib/novanix.h"
#include "../lib/a.out.h"

#define MAX_LINE 256  // Maximum number of tokens per file line
#define MAX_STR  256  // Maximum length of user defined strings
#define PAGESIZE 1024 // 2 KB (1 KW) of memory (1 mmu page)

// +--------------+
// | TOKEN VALUES |
// +--------------+
// Start at 128 so that we don't overlap ASCII tokens
enum
{
    Number = 128, LongNumber, // Numerical constant in source
    Named, Symbol, Nspace, // User defined symbols/types
// ******* Reserved words *******
    // *** Specifiers and qualifiers ***
    Void, Int, Short, Char, Float, Long, // Storage types
    Enum, Struct, Union, // Data structures
    Auto, Static, Register, Const, // Storage classes
    Extern, Signed, Unsigned, // Storage Qualifiers
    // *** Program structure ***
    If, Else, Case, Default, // Branch
    Break, Continue, Return, // Terminate
    For, While, Do, // Loops
    Sizeof, Variadic, // Compiler
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
    LogNot, Not, Inc, Dec, // Logical not, bitwise not, increment and decrement
    Brak, Dot, Arrow // Square bracket (array access), Dot (structure access), Arrow (structure access via pointer)
};

// Namespace attributes bitmask

// |0000|0|000
// |    | |Namespace type

#define CPL_BLOC 0 // File or Block scope
#define CPL_STRC 1 // Struct
#define CPL_UNIN 2 // Union
#define CPL_ENUM 3 // Enumeration
#define CPL_FUNC 4 // Function arguments
#define CPL_VFUNC 5 // Variadic Function arguments
#define CPL_NSPACE_MASK 7

// NameSPace
struct mccnsp
{
    char * name; // NULL if anonymous
    unsigned char len; // Length of name
    unsigned char type; // Type of namespace

    unsigned int size; // Number of bytes occupied by member symbols

    struct mccsym * symtbl, ** symtail; // Member symbols
    struct mccnsp * nsptbl, ** nsptail; // Child namespaces

    struct mccnsp * parent; // Parent namespace

    struct mccnsp * next; // Next namespace in this list
};

struct mccfunc
{
    unsigned int size; // Function array size
    unsigned char inder; // Number of indirections

    unsigned char argc;

    struct mccnsp * argnsp; // Argument namespace
};

// Data type bitmask

// |00|0|00|000
// |  | |  |Primative datatype
// |  | |Storage type
// |  |Extern flag

#define CPL_VOID 0  // ( 0 bits) Void
#define CPL_CHR  1  // ( 8 bits) Signed Character
#define CPL_UCHR 2  // ( 8 bits) Unsigned Character
#define CPL_INT  3  // (16 bits) Signed Integer
#define CPL_UINT 4  // (16 bits) Unsigned Integer
#define CPL_LNG  5  // (32 bits) Signed Long
#define CPL_ULNG 6  // (32 bits) Unsigned Long
#define CPL_FPV  7  // (32 bits) Float
#define CPL_DTYPE_MASK 7 // Mask

// C Storage types
#define CPL_TEXT 0 << 3 // Text (write only) (functions and constants)
#define CPL_ZERO 1 << 3 // Zero page (register)
#define CPL_DATA 2 << 3 // Data (static)
#define CPL_STAK 3 << 3 // Stack (dynamic)
#define CPL_STORE_MASK 3 << 3 // Mask

// C Storage qualifiers
#define CPL_XTRN 1 << 5 // Extern flag

struct mcctype
{
    unsigned char type; // Primative type data
    unsigned char inder; // Number of inderections (in addition to the one implied by size)

    unsigned int size; // Number of elements in array (anything but 0 implies one level of inderection)

    struct mccfunc * ftype; // NULL if not a function

    struct mccnsp * stype; // Struct type (Overrides primative datatype)
};

// Symbol
struct mccsym
{
    char * name;
    unsigned char len;

    struct mcctype type; // What type is this symbol?

    struct mccsym * next; // Store the next symbol in the table
};

// Store a data segment
struct segment
{
    int fd;
    unsigned int pos;
    unsigned int size;
};

// Assembly fail function
void mccfail(char * msg);

void define(struct mccnsp * curnsp);
