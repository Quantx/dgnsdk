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

// *** Memory Operations ***
    // [dst] -> src
    OpLoadByte,
    OpLoadWord,
    OpLoadLong,

    // src -> [dst]
    OpStoreByte,
    OpStoreWord,
    OpStoreLong,

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
    OpCall, OpReturn,

// *** Memory instructions (use same format as computational instructions)
    // [dst] -> src
    OpLoadByte,
    OpLoadWord,
    OpLoadLong,

    // src -> [dst]
    OpStoreByte,
    OpStoreWord,
    OpStoreLong,

// *** Computational operations
    // Logical operations
    OpLogOr, OpLogAnd, OpLogNot,
    OpEq, OpNeq,
    OpLess, OpLessEq, OpGreat, OpGreatEq,
    // Bitwise Operations
    OpOr, OpAnd, OpXor, OpNot,
    OpShl, OpShr,
    // Arithmetic Operations
    OpAdd, OpSub,
    OpMul, OpDiv, OpMod,
    OpNegate,
    OpInc, OpDec
};

struct mccoper_value // Load an immediate value
{
    // Value
    union {
        int16_t val;
        int32_t valLong;
    }
    // Register address
    unsigned int8_t reg;
}

struct mccoper_addrglb // Load address of global variable
{
    // Variable name
    int8_t * name;
    // Variable name length
    unsigned int8_t len;
    // Register address
    unsigned int8_t reg;
};


struct mccoper_addrloc // Load address of local variable
{
    // Stack offset
    unsigned int16_t mem;
    // Register address
    unsigned int8_t reg;
};

struct mccoper_statement
{
    // Unique statment id used for generating things like unique jmp labels
    unsigned int32_t id;
};

/*
    Larger argument is always the destination (size) in bytes
    Thus: src <= dst

    Example:
    left (int) + right (long)
    right must be destination since left is too small
*/
struct mccoper_computational
{
    // Source register
    unsigned int8_t src;
    // Destination register
    unsigned int8_t dst;
    // Size of destination register
    unsigned int16_t size;
};

struct mccoper
{
    unsigned int8_t op;
    unsigned int8_t flag;

    union {
        struct mccoper_value v;
        struct mccoper_addrglb g;
        struct mccoper_addrloc l;
        struct mccoper_statement s;
        struct mccoper_computational c;
    };
};
