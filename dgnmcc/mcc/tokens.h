// +--------------+
// | TOKEN VALUES |
// +--------------+
enum
{
// ******* Reserved words *******
    EndOfFile,
    // *** Specifiers and qualifiers ***
    Void, Int, Short, Char, Float, Long, Double, // Storage types
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
    Variadic,           // '...'
    SemiColon,          // ';'
    Q_Mark, Colon,      // '?' ':'
    Square_L, Square_R, // '[' ']'
    Curly_L, Curly_R,   // '{' '}'
    Paren_L, Paren_R,   // '(' ')'
// ******* Intermediate Representation Tokens *******
    End, // End of block
    Allocate, Unallocate, // Allocate memory on the stack
    Label, LabelExtern, // An assembly label
    VariableLocal, // Local variable on the stack

// ******* Expression tokens *******
    Named, Variable, // Named (an identifier string), Variable (ref to user defiend variable)
    String, // String constant
    Number, SmolNumber, LongNumber, FpvNumber, DblNumber, // Numerical constant in source (Char, Int, Long, Float, Double)
// ******* Operators *******
    Comma, Arg,
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
    "EOF",
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

    ";",
    "?", ";",
    "[", "]",
    "{", "}",
    "(", ")",

    "End",
    "Allocate", "Unallocate",
    "Label", "LabelExtern",
    "VariableStack",

    "named", "variable",
    "string",
    "number", "small number", "long number", "float number", "double number",

    ",", ", (Func Arg)",
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
