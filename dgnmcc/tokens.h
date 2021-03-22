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
#define IR_VOID CPL_VOID
#define IR_CHR  CPL_CHR
#define IR_UCHR CPL_UCHR
#define IR_INT  CPL_INT
#define IR_UINT CPL_UINT
#define IR_LNG  CPL_LNG
#define IR_ULNG CPL_ULNG
#define IR_FPV  CPL_FPV
#define IR_DBL  CPL_DBL
// Complex types
#define IR_STRUC 9
#define IR_UNION 10
// Derived types
#define IR_PTR   11
#define IR_FUNC  12

#define IR_TYPE_MASK 0b00001111



struct mccirn
{
    unsigned int8_t oper;
    unsigned int8_t flag;
    union {
        unsigned int16_t size;
	int16_t val;
	int32_t valLong;
    };
};
