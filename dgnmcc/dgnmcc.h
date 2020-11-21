#include "../lib/novanix.h"
#include "../lib/a.out.h"

#define MAX_LINE      256  // Maximum number of tokens per file line
#define MAX_STR       256  // Maximum length of user defined strings
#define PAGESIZE      1024 // 2 KB (1 KW) of memory (1 mmu page)
#define MAX_EXPR_OPER 64   // Maximum size of the expression operator stack
#define MAX_EXPR_NODE 256  // Maximum size of the expression node stack

// +--------------+
// | TOKEN VALUES |
// +--------------+
// Start at 128 so that we don't overlap ASCII tokens
enum
{
// ******* Reserved words *******
    // *** Specifiers and qualifiers ***
    Void = 128, Int, Short, Char, Float, Long, // Storage types
    Enum, Struct, Union, // Data structures
    Auto, Static, Register, Const, // Storage classes
    Extern, Signed, Unsigned, // Storage Qualifiers
    // *** Program structure ***
    If, Else, Case, Default, // Branch
    Break, Continue, Return, // Terminate
    For, While, Do, // Loops
    Goto, // Blasphemy
    Sizeof, // Technically an operator
// ******* Misc *******
    Variadic,
// ******* Expression tokens *******
    Number, LongNumber, // Numerical constant in source
    Named, //Symbol, Nspace, // User defined symbols/types
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
    LogNot, Not, Plus, Minus, Inder, Deref, PreInc, PreDec, // Logical not, bitwise not, increment, decrement
    Brak, Dot, Arrow, PostInc, PostDec // Open square bracket (array access), Dot (structure access), Arrow (structure access via pointer)
};

/* Namespace attributes bitmask

|0000|0|000
|    | |Namespace type
|    |Defined flag

*/

#define CPL_BLOC 0 // Block scope
#define CPL_STRC 1 // Struct
#define CPL_UNIN 2 // Union
#define CPL_ENUM 3 // Enumeration
#define CPL_FUNC 4 // Function arguments
#define CPL_VFUNC 5 // Variadic Function arguments
#define CPL_NSPACE_MASK 7

#define CPL_DEFN 8 // Defined flag

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

/* Type example (because I keep forgetting)

int (*test)();

mccsym->name = test;
mccsym->ptype = int;

mccsym->type = mcctype1

mcctype1->inder = 1
mcctype1->type = mcctype2

mcctype2->ftype = mccnsp

*/

// Type information
struct mcctype
{
    // Total number of indirections = inder + arrays;
    unsigned char inder; // Number of inderections
    unsigned char arrays; // Number of sizes

    unsigned int * sizes; // List of array sizes

    struct mccnsp * ftype; // Function namespace (NULL if not a function)
    struct mcctype * type; // Sub type
};

/* Data type bitmask

|0|0|00|0000
| | |  |Primative datatype
| | |Storage type
| |Extern flag

*/

#define CPL_VOID 0  // ( 0 bits) Void
#define CPL_CHR  1  // ( 8 bits) Signed Character
#define CPL_UCHR 2  // ( 8 bits) Unsigned Character
#define CPL_INT  3  // (16 bits) Signed Integer
#define CPL_UINT 4  // (16 bits) Unsigned Integer
#define CPL_LNG  5  // (32 bits) Signed Long
#define CPL_ULNG 6  // (32 bits) Unsigned Long
#define CPL_FPV  7  // (32 bits) DG Nova Float
#define CPL_DBL  8  // (64 bits) DG Nova Double
#define CPL_DTYPE_MASK 8 // Mask

// C Storage types
#define CPL_TEXT 0 << 4 // Text (write only) (functions and constants)
#define CPL_ZERO 1 << 4 // Zero page (register)
#define CPL_DATA 2 << 4 // Data (static)
#define CPL_STAK 3 << 4 // Stack (dynamic)
#define CPL_STORE_MASK 3 << 4 // Mask

// C Storage qualifiers
#define CPL_XTRN 1 << 6 // Extern flag

// Symbol
struct mccsym
{
    char * name;
    unsigned char len;

    unsigned char ptype; // Primative datatype
    struct mccnsp * stype; // Struct type

    unsigned int addr;

    struct mcctype type; // Complex type information

    struct mccsym * next; // Store the next symbol in the table
};

// Tree node
struct mccnode
{
    unsigned char type; // What operation or datatype does this node represent

    // Either a constant value, or a symbol
    union {
        unsigned int val;
        unsigned long valLong;
        struct mccsym * sym;
    };

    struct mccnode * left, * right;
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
