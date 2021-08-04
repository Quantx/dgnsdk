enum
{
// *** Global Memory operations ***
    // Load a global constant value (expects a label)
    OpGlbConstByte,
    OpGlbConstWord,
    OpGlbConstLong,

// *** Local memory operations ***
    // Load a local constant value (expects a stack offset)
    OpLocConstByte,
    OpLocConstWord,
    OpLocConstLong,

// *** Transfer a register (src) to the memory location indicated by a pointer in a register (dst)
    OpLoadByte, OpStoreByte,
    OpLoadWord, OpStoreWord,
    OpLoadLong, OpStoreLong,

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

struct mccoper_glbmem // Load constant or pointer from global memory
{
    // Variable name
    int8_t * name;
    int8_t len;
    int8_t reg;
};


struct mccoper_locmem // Load constant or pointer from local memory
{
    // Stack offset
    int16_t mem;
    // Register address
    int8_t reg;
};

struct mccoper_statement
{
    // Unique statment id used for generating things like unique jmp labels
    unsigned int32_t id;
};

/*
    Larger argument is always the destination:
    left (int) + right (long)
    right must be destination since left is too small
*/
struct mccoper_computational
{
    int8_t src;
    int8_t dst;
};

struct mccoper
{
    unsigned int8_t op;
    unsigned int8_t flag;

    union {
        struct mccoper_glbmem g;
        struct mccoper_locmem l;
        struct mccoper_statement s;
        struct mccoper_computational c;
    };
};
