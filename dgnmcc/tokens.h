// +--------------+
// | TOKEN VALUES |
// +--------------+
// Start at 128 so that we don't overlap ASCII tokens
enum
{
// ******* Reserved words *******
    // *** Specifiers and qualifiers ***
    Void = 128, Int, Short, Char, Float, Long, Double, // Storage types
    Enum, Struct, Union, // Data structures
    Signed, Unsigned, // Storage Qualifiers
    Extern,
    Auto, Static, Register, Const, // Storage classes
    // *** Program structure ***
    If, Else, Switch, Case, Default, // Branch
    Break, Continue, Return, // Terminate
    For, While, Do, // Loops
    Goto, // Blasphemy
    SizeofRes, // Used only for identifying the reserved word, not be confused with Sizeof
// ******* Misc *******
    Variadic,

// ******* Intermediate Representation Tokens *******
    End, // End of block
    Allocate, Unallocate, // Allocate memory on the stack
    Label, LabelExtern, // An assembly label
    VariableStack, // Variable on the stack

// ******* Expression tokens *******
    Named, Variable, // Named (an identifier string), Variable (ref to user defiend variable)
    Number, SmolNumber, LongNumber, FpvNumber, DblNumber, // Numerical constant in source (Char, Int, Long, Float, Double)
// ******* Operators *******
    Comma,
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
    Sizeof, Cast, LogNot, Not, Plus, Minus, Inder, Deref, PreInc, PreDec, // Sizeof, logical not, bitwise not, Pre-Increment, Pre-Decrement
    PostInc, PostDec, Dot, Arrow, FnCallArgs, FnCall // Post-Increment, Post-Decrement, Dot (structure access), Arrow (structure access via pointer)
};

#ifdef DEBUG
int8_t * tokenNames[] = {
    "void", "int", "short", "char", "float", "long", "double",
    "enum", "struct", "union",
    "signed", "unsigned",
    "extern",
    "auto", "static", "register", "const",
    "if", "else", "switch", "case", "default",
    "break", "continue", "return",
    "for", "while", "do",
    "goto",
    "sizeof (reserved word)",
    "...",

    "End",
    "Allocate", "Unallocate",
    "Label", "LabelExtern",
    "VariableStack",

    "named", "variable",
    "number", "small number", "long number", "float number", "double number",

    ",",
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
    "sizeof (operator)", "cast", "!", "~", "(unary) +", "(unary) -", "(unary) &", "(unary) *", "(pre) ++", "(pre) --",
    "(post) ++", "(post) --", ".", "->", "call (w. args)", "call"
};
#endif

// Primative types
#define IR_VOID 0 // ( 0 bits) Void
#define IR_CHR  1 // ( 8 bits) Signed Character
#define IR_UCHR 2 // ( 8 bits) Unsigned Character
#define IR_INT  3 // (16 bits) Signed Integer
#define IR_UINT 4 // (16 bits) Unsigned Integer
#define IR_LNG  5 // (32 bits) Signed Long
#define IR_ULNG 6 // (32 bits) Unsigned Long
#define IR_FPV  7 // (32 bits) DG Nova Float
#define IR_DBL  8 // (64 bits) DG Nova Double
// Complex types
#define IR_STRUC 9
// Derived types
#define IR_PTR   10
#define IR_ARRAY 11
#define IR_FUNC  12

#define IR_TYPE_MASK 0b00001111

#define IR_LVAL (1 << 4) // Flag is set if this NODE is an L-value


// Intermediate Represntation Compiler Node
struct ircnode
{
    unsigned int8_t oper;
    unsigned int8_t type;
    unsigned int16_t size; // Size of pointer
    union {
        struct {
            unsigned int16_t val;
            int8_t * name;
        };
	unsigned int32_t valLong;
    };
};
