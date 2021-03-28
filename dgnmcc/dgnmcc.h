#include "../lib/unistd.h"

#ifdef DEBUG
#define DEBUG_TOKEN 1
#define DEBUG_EXPR 1
#define DEBUG_DECLARE 1
#define DEBUG_TYPECHECK 1
#endif

#define MAX_LINE      256  // Maximum number of tokens per file line
#define MAX_STR       256  // Maximum length of user defined strings
#define PAGESIZE      1024 // 2 KB (1 KW) of memory (1 mmu page)

#define MAX_EXPR_OPER 64   // Maximum size of the expression operator stack
#define MAX_EXPR_NODE 256  // Maximum size of the expression node stack
#define MAX_EXPR_CAST 16    // Maximum number of casts in an expression

#include "tokens.h"

#ifdef DEBUG
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
    "enum_constant"
};

int8_t * namespaceNames[] = {
    "code_block",
    "struct",
    "union",
    "enum",
    "func",
    "variadic_function",
    "casting_namespace"
};
#endif

/* Namespace attributes bitmask

|000|0|0|000
|   | | |Namespace type
|   | |Defined flag
|   |Instantiated flag (at lease one variable was declared using this namespace as it's type)

*/

#define CPL_BLOCK 0 // Block scope
#define CPL_STRC  1 // Struct
#define CPL_UNION 2 // Union
#define CPL_ENUM  3 // Enumeration
#define CPL_FUNC  4 // Function arguments
#define CPL_VFUNC 5 // Variadic Function arguments
#define CPL_CAST  6 // Special namespace used for casting
#define CPL_NSPACE_MASK 0b111

#define CPL_DEFN (1 << 3) // Defined flag
#define CPL_INST (1 << 4) // This flag is set when a struct is instantiated

// NameSPace
struct mccnsp
{
    int8_t * name; // NULL if anonymous
    unsigned int8_t len; // Length of name

    unsigned int8_t type; // Type of namespace

    unsigned int16_t addr; // Base address offset for this namespace (used by anonymous namespaces)
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

/* Data type bitmask (stored in: type->ptype)

|0|000|0000
| |   |Primative datatype
| |Storage type
|Local flag

*/

#define CPL_VOID IR_VOID // ( 0 bits) Void
#define CPL_CHR  IR_CHR  // ( 8 bits) Signed Character
#define CPL_UCHR IR_UCHR // ( 8 bits) Unsigned Character
#define CPL_INT  IR_INT  // (16 bits) Signed Integer
#define CPL_UINT IR_UINT // (16 bits) Unsigned Integer
#define CPL_LNG  IR_LNG  // (32 bits) Signed Long
#define CPL_ULNG IR_ULNG // (32 bits) Unsigned Long
#define CPL_FPV  IR_FPV  // (32 bits) DG Nova Float
#define CPL_DBL  IR_DBL  // (64 bits) DG Nova Double
#define CPL_ENUM_CONST 9 // (16 bits) Enum constant

#define CPL_DTYPE_MASK 0b00001111 // Mask

// Segments
#define SEG_ZERO 0
#define SEG_TEXT 1
#define SEG_CNST 2
#define SEG_DATA 3
#define SEG_BSS  4
#define SEG_STAK 5

// C Storage types
#define CPL_ZERO (SEG_ZERO << 4) // Zero page (register)
#define CPL_TEXT (SEG_TEXT << 4) // Text (function code)
#define CPL_CNST (SEG_CNST << 4) // Constants (read only data)
#define CPL_DATA (SEG_DATA << 4) // Data (static)
#define CPL_BSS  (SEG_BSS  << 4) // Bss  (initialized to zero)
#define CPL_STAK (SEG_STAK << 4) // Stack (dynamic)
#define CPL_STORE_MASK (0b01110000) // Mask

// C Storage qualifiers
#define CPL_LOCAL 0b10000000 // Local flag, not set when declared as "extern int myint;"

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

    unsigned int16_t addr;

    struct mcctype type; // Type information

    struct mccsym * next; // Store the next symbol in the table
};

#define CPL_LVAL 1 // True if this node contains an l-value

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

// Fix circular dependancy
void define(struct mccnsp * curnsp);

// Compiler fail function
void mccfail(int8_t * msg);

#ifdef DEBUG
void dumpNamespace( int16_t fd, struct mccnsp * dmpnsp );
void writeType(int16_t fd, struct mcctype * t, int8_t * name, int16_t len);
void writeToken(int16_t fd, unsigned int8_t tokn);
void dumpTree(struct mccnode * n, int8_t * fname);
void dumpGlbnsp( int8_t * fname );
#endif
