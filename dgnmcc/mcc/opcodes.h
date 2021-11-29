#define OP_CLASS_VOID 0
#define OP_CLASS_SIGNED 1
#define OP_CLASS_UNSIGNED 2
#define OP_CLASS_FLOAT 3
#define OP_CLASS_POINTER 4

enum
{
// *** Immediate Values ***
    // Load a immediate value (value is embeded into opcode)
    OpValueByte,
    OpValueWord,
    OpValueLong,

    // Load the address of a global variable (expects a label)
    OpAddrGlb,
    // Load the address of a local variable (expects a stack offset)
    OpAddrLoc,

// *** Statements ***
    OpEnd,
    // Conditional
    OpIf, OpElse,
    // Loop
    OpDo, OpWhile, OpFor,
    // Switch
    OpSwitch, OpCase, OpDefault,
    // Flow
    OpBreak, OpContinue,
    // Function
    OpArg, OpCall, OpReturn,

// *** Memory instructions (use same format as computational instructions)
    // [r_arg] -> reg
    OpLoad,

    // reg -> [r_arg]
    OpStore,
    
    // reg -> [stack_top]
    OpPush,
    
    // [stack_top] -> reg
    OpPop,
    
// *** Computational operations ( S:signed, U:unsigned, F:float, P:pointer )
    // Logical operations
    OpLogOr, OpLogAnd, OpLogNot,
    OpEq, OpNeq,
    OpLess_S, OpLessEq_S, OpGreat_S, OpGreatEq_S,
    OpLess_U, OpLessEq_U, OpGreat_U, OpGreatEq_U, // Unsigned is also used for pointers
    OpLess_F, OpLessEq_F, OpGreat_F, OpGreatEq_F,
    // Bitwise Operations
    OpOr, OpAnd, OpXor, OpNot,
    OpShl, OpShr_S, OpShr_U, // Shift ops don't apply to floating point

    // Arithmetic Operations
    OpAdd, OpSub,
    OpAdd_P, OpSub_P, OpSub_PP, // OpSub_PP occurs when both rhs & lhs are pointers

    OpMul_S, OpDiv_S, OpMod_S,
    OpMul_U, OpDiv_U, OpMod_U,
    OpMul_F, OpDiv_F, // (OpMod_F isn't supported)

    OpNegate, OpNegate_F,
    
    OpPreInc, OpPreDec,
    OpPreInc_F, OpPreDec_F,
    OpPostInc, OpPostDec,
    OpPostInc_F, OpPostDec_F,
    
    // Floating point and int conversion 
    OpFloat2Int, OpInt2Float,
    OpFloat2Float, // Used for converting between floats and doubles

    // Structure Operators
    OpArrow, OpDot, OpArray
};

#ifdef DEBUG
int8_t * opNames[] = {
    "OpValueByte",
    "OpValueWord",
    "OpValueLong",

    "OpAddrGlb",

    "OpAddrLoc",

    "OpEnd",

    "OpIf", "OpElse",

    "OpDo", "OpWhile", "OpFor",

    "OpSwitch", "OpCase", "OpDefault",

    "OpBreak", "OpContinue",

    "OpArg", "OpCall", "OpReturn",

    "OpLoad",

    "OpStore",
    
    "OpPush",
    
    "OpPop",
    
    "OpLogOr", "OpLogAnd", "OpLogNot",
    "OpEq", "OpNeq",
    "OpLess_S", "OpLessEq_S", "OpGreat_S", "OpGreatEq_S",
    "OpLess_U", "OpLessEq_U", "OpGreat_U", "OpGreatEq_U",
    "OpLess_F", "OpLessEq_F", "OpGreat_F", "OpGreatEq_F",

    "OpOr", "OpAnd", "OpXor", "OpNot",
    "OpShl", "OpShr_S", "OpShr_U",

    "OpAdd", "OpSub",
    "OpAdd_P", "OpSub_P", "OpSub_PP",

    "OpMul_S", "OpDiv_S", "OpMod_S",
    "OpMul_U", "OpDiv_U", "OpMod_U",
    "OpMul_F", "OpDiv_F",

    "OpNegate", "OpNegate_F",
    
    "OpPreInc", "OpPreDec",
    "OpPreInc_F", "OpPreDec_F",
    "OpPostInc", "OpPostDec",
    "OpPostInc_F", "OpPostDec_F",

    "OpFloat2Int", "OpInt2Float",
    "OpFloat2Float",

    "OpArrow", "OpDot", "OpArray"
};
#endif

struct mccoper_value // Load an immediate value
{
    // Value
    union {
        int16_t val;
        int32_t valLong;
    };
};

struct mccoper_addr // Load address of global/local variable
{
    // Variable name
    int8_t * name;
    // Variable name length or local stack-address offset
    unsigned int16_t val;
};

struct mccoper_statement
{
    // Unique statment id used for generating things like unique jmp labels
    unsigned int16_t id;
};

struct mccoper_computational
{
    // Source register
    unsigned int8_t l_arg;
    // Destination register
    unsigned int8_t r_arg;
    // Size of source register
    unsigned int16_t l_size;
    // Size of destination register
    unsigned int16_t r_size;
};

struct mccoper
{
    unsigned int8_t op;
    unsigned int8_t reg;
    unsigned int16_t size;

    union {
        struct mccoper_value v;
        struct mccoper_addr a;
        struct mccoper_statement s;
        struct mccoper_computational c;
    };
};

// Gets the actual size in bytes of a register (decodes FPV/DBL and other special values)
unsigned int16_t regSize(unsigned int16_t size);
