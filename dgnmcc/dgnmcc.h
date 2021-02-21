#include "../lib/novanix.h"
#include "../lib/a.out.h"

#define DEBUG 1

#ifdef DEBUG
#define DEBUG_TOKEN 1
#define DEBUG_EXPR 1
#endif

#define MAX_LINE      256  // Maximum number of tokens per file line
#define MAX_STR       256  // Maximum length of user defined strings
#define PAGESIZE      1024 // 2 KB (1 KW) of memory (1 mmu page)
#define MAX_EXPR_OPER 64   // Maximum size of the expression operator stack
#define MAX_EXPR_NODE 256  // Maximum size of the expression node stack
#define MAX_SUB_TYPE  8    // Maximum number of subtypes per type

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
    SizeofRes, // Used only for identifying the reserved word, not be confused with Sizeof
// ******* Misc *******
    Variadic,
// ******* Expression tokens *******
    Number, SmolNumber, LongNumber, // Numerical constant in source (Char, Int, Long)
    Named, Variable, // Named (an identifier string), Variable (ref to user defiend variable)
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
    Sizeof, LogNot, Not, Plus, Minus, Inder, Deref, PreInc, PreDec, // Sizeof, logical not, bitwise not, Pre-Increment, Pre-Decrement
    PostInc, PostDec, Dot, Arrow // Post-Increment, Post-Decrement, Dot (structure access), Arrow (structure access via pointer)
};

#ifdef DEBUG
int8_t * tokenNames[] = {
    "void", "int", "short", "char", "float", "long",
    "enum", "struct", "union",
    "auto", "static", "register", "const",
    "extern", "signed", "unsigned",
    "if", "else", "case", "default",
    "break", "continue", "return",
    "for", "while", "do",
    "goto",
    "sizeof (reserved word)",
    "...",
    "number", "small number", "long number",
    "named", "variable",

    "=", "+=", "-=", "*=", "/=", "%=", "<<=", ">>=", "&=", "^=", "|=",
    "?",
    "||",
    "&&",
    "|",
    "^",
    "&",
    "==", "!=",
    "<", "<=", ">", ">=",
    "<<", ">>",
    "+", "-",
    "*", "/", "%",
    "sizeof (operator)", "!", "~", "(unary) +", "(unary) -", "(unary) &", "(unary) *", "(pre) ++", "(pre) --",
    "(post) ++", "(post) --", ".", "->"
};

int8_t * typeNames[] = {
    "void",
    "char",
    "uchar",
    "int",
    "uint",
    "long",
    "ulong",
    "float",
    "double",
    "struct",
    "union",
    "enum"
};
#endif

/* Namespace attributes bitmask

|0000|0|000
|    | |Namespace type
|    |Defined flag

*/

#define CPL_BLOCK 0 // Block scope
#define CPL_STRC  1 // Struct
#define CPL_UNION 2 // Union
#define CPL_ENUM  3 // Enumeration
#define CPL_FUNC  4 // Function arguments
#define CPL_VFUNC 5 // Variadic Function arguments
#define CPL_NSPACE_MASK 7

#define CPL_DEFN 8 // Defined flag

// NameSPace
struct mccnsp
{
    int8_t * name; // NULL if anonymous
    unsigned int8_t len; // Length of name

    unsigned int8_t type; // Type of namespace

    unsigned int16_t size; // Number of bytes occupied by member symbols

    struct mccsym * symtbl, ** symtail; // Member symbols
    struct mccnsp * nsptbl, ** nsptail; // Child namespaces

    struct mccnsp * parent; // Parent namespace

    struct mccnsp * next; // Next namespace in this list
};

/* Type examples (because I keep forgetting)

*** Function pointer ***

long (*test)();

mccsym->name = "test";
mccsym->ptype = CPL_INT;

mccsym->type = mcctype1

mcctype1->inder = 1
mcctype1->type = mcctype2

mcctype2->ftype = mccnsp_args // Function argument namespace

*** Function declaration ***

struct myvar * test( ) { ...some code... }

mccsym->name = "test"

mccsym->stype = ptr_to_myvar_symbol

mccsym->type = mcctype

mcctype->inder = 1;
mcctype->ftype = mccnsp_args // Function argument namespace

mccnsp_args->nsptbl = mccnsp_code // Code namespace is a child
*/

/* Data type bitmask

|0|0|00|0000
| | |  |Primative datatype
| | |Storage type
| |Extern flag

*/

#define CPL_VOID 0 // ( 0 bits) Void
#define CPL_CHR  1 // ( 8 bits) Signed Character
#define CPL_UCHR 2 // ( 8 bits) Unsigned Character
#define CPL_INT  3 // (16 bits) Signed Integer
#define CPL_UINT 4 // (16 bits) Unsigned Integer
#define CPL_LNG  5 // (32 bits) Signed Long
#define CPL_ULNG 6 // (32 bits) Unsigned Long
#define CPL_FPV  7 // (32 bits) DG Nova Float
#define CPL_DBL  8 // (64 bits) DG Nova Double
#define CPL_DTYPE_MASK 15 // Mask

// C Storage types
#define CPL_TEXT 0 << 4 // Text (write only) (functions and constants)
#define CPL_ZERO 1 << 4 // Zero page (register)
#define CPL_DATA 2 << 4 // Data (static)
#define CPL_STAK 3 << 4 // Stack (dynamic)
#define CPL_STORE_MASK 3 << 4 // Mask

// C Storage qualifiers
#define CPL_XTRN 1 << 6 // Extern flag

// Type information
struct mccsubtype
{
    // Total number of indirections = inder + arrays;
    unsigned int8_t inder; // Number of inderections
    unsigned int8_t arrays; // Number of sizes

    unsigned int16_t * sizes; // List of array sizes

    struct mccnsp * ftype; // Function namespace (NULL if not a function)
    struct mccsubtype * sub; // Sub type
};

struct mcctype
{
    unsigned int8_t ptype; // Primative datatype
    struct mccnsp * stype; // Struct type
    struct mccsubtype * sub; // Complex type information (MUST NEVER BE NULL)
};

// Symbol
struct mccsym
{
    int8_t * name;
    unsigned int8_t len;

    unsigned int16_t addr; // Offset from start of segment
    struct mcctype type; // Type information

    struct mccsym * next; // Store the next symbol in the table
};

#define CPL_LVAL 1 << 0 // True if this node contains an l-value

// Tree node
struct mccnode
{
    unsigned int8_t oper; // What operation does this node represent
    unsigned int8_t flag; // Flags for this node

    struct mcctype * type; // Type information

    // Leafs are either a constant value, or a named symbol
    union {
        struct {
            unsigned int16_t val;
            int8_t * name;
        };
        struct mccsym * sym;
        unsigned int32_t valLong;
    };

    struct mccnode * left, * right;
};

// Store a data segment
struct segment
{
    int16_t fd;
    unsigned int16_t pos;
    unsigned int16_t size;
};

// Compiler fail function
void mccfail(int8_t * msg);

void define(struct mccnsp * curnsp);

#ifdef DEBUG
void writeToken(int16_t fd, unsigned int8_t tokn);
void dumpTree(struct mccnode * n, int8_t * fname);
#endif
