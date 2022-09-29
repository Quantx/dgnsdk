#define OP_CLASS_VOID 0
#define OP_CLASS_SIGNED 1
#define OP_CLASS_UNSIGNED 2
#define OP_CLASS_FLOAT 3
#define OP_CLASS_POINTER 4

enum
{
    OpNop, // No-operation, should be first in this list
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
    OpStart, OpEnd,
    // Conditional
    OpIf, OpElse,
    // Loop
    OpFor, OpWhile, OpDo,
    // Switch
    OpSwitch, OpCase, OpDefault,
    // Flow
    OpBreak, OpContinue,
    // Function
    OpReturn,

// *** Memory instructions (use same format as computational instructions)
    // [r_arg] -> reg
    // r_size = Maximum number of bytes to read from memory
    // size = Number of registers to fill (number of bytes to actually read)
//    OpLoad,

    // reg -> [r_arg]
    // Reg must be the source register because the value may be referenced by additional statements
    // r_size = Maximum number of bytes to write to memory
    // size = Number of registers to write (number of bytes to actually write)
//    OpStore,
    
    // reg -> [stack_top]
    OpPush,
    
    // [stack_top] -> reg
    OpPop,
    
// *** Computational operations ( S:signed, U:unsigned, F:float, P:pointer )
    // Function operations
    OpArg, OpCall,
    // r_arg -> reg (If r_arg == reg then this is a no-op)
    OpMov,
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

    // Indirect Pre/Pos Inc/Dec
/*
    OpPreIncLoad,    OpPreDecLoad,
    OpPreIncLoad_F,  OpPreDecLoad_F,
    OpPostIncLoad,   OpPostDecLoad,
    OpPostIncLoad_F, OpPostDecLoad_F,
*/
    // Direct Pre/Pos Inc/Dec
    OpPreInc,    OpPreDec,
    OpPreInc_F,  OpPreDec_F,
    OpPostInc,   OpPostDec,
    OpPostInc_F, OpPostDec_F,
    
    // Floating point and int conversion 
    OpFloatToInt, OpIntToFloat,
    OpFloatToFloat, // Used for converting between floats and doubles

    // Structure Operators
    OpArrow, OpDot
#ifdef DEBUG
    , _OpListCount
#endif
};

#ifdef DEBUG
int8_t * opNames[] = {
    "OpNop",

    "OpValueByte",
    "OpValueWord",
    "OpValueLong",

    "OpAddrGlb",

    "OpAddrLoc",

    "OpStart", "OpEnd",

    "OpIf", "OpElse",

    "OpFor", "OpWhile", "OpDo",

    "OpSwitch", "OpCase", "OpDefault",

    "OpBreak", "OpContinue",

    "OpReturn",

//    "OpLoad",

//    "OpStore",
    
    "OpPush",
    
    "OpPop",
    
    "OpArg", "OpCall",
    
    "OpMov",
    
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
/*
    "OpPreIncLoad", "OpPreDecLoad",
    "OpPreIncLoad_F", "OpPreDecLoad_F",
    "OpPostIncLoad", "OpPostDecLoad",
    "OpPostIncLoad_F", "OpPostDecLoad_F",
*/
    "OpPreInc", "OpPreDec",
    "OpPreInc_F", "OpPreDec_F",
    "OpPostInc", "OpPostDec",
    "OpPostInc_F", "OpPostDec_F",

    "OpFloatToInt", "OpIntToFloat",
    "OpFloatToFloat",

    "OpArrow", "OpDot"
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
    // Sub-id used for case statements
    unsigned int16_t sid;
};

struct mccoper_computational
{
    // Source register
    unsigned int8_t l_arg;
    // Destination register
    unsigned int8_t r_arg;
    // Size of source register
    int16_t l_size;
    // Size of destination register
    int16_t r_size;
};

// Inderection flags indicate that the desired register contains a pointer to desired data, not the data itself
#define REG_INDER   0b001
#define R_ARG_INDER 0b010
#define L_ARG_INDER 0b100

struct mccoper
{
    unsigned int8_t op;
    unsigned int8_t reg;
    unsigned int16_t flags;
    int16_t size;

    union {
        struct mccoper_value v;
        struct mccoper_addr a;
        struct mccoper_statement s;
        struct mccoper_computational c;
    };
};

// Gets the actual size in bytes of a register (decodes FPV/DBL and other special values)
unsigned int16_t regSize(unsigned int16_t size);
