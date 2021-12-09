#include "../../lib/unistd.h"
#include "../mcc/tokens.h"
#include "../mcc/statements.h"

#ifdef DEBUG
#define DEBUG_NODE 1
#endif

#define MAX_STATEMENT 64
#define MAX_EVAL_STK 64 // Maximum size (in words) of the zero page expression evaluation stack
#define MAX_OPER_BUF 256 // Maximum number of instructions per statement
#define MAX_STATEMENT_STK 64 // Maximum depth of statement stack depth

#define MAX_ANALYSIS_BRK 4096 // Number of bytes for analysis brk region

#include "../mcc/opcodes.h"

// Compiler fail function
void mccfail(int8_t * msg);

// Used to pair expression nodes with their corresponding opcodes
struct mcceval
{
    struct mccoper * op;
    struct mccstmt * st;
    struct mcceval * left, * right, * parent;
};

struct mccvar
{
    union
    {
        int8_t * name; // Global variable
        unsigned int16_t addr; // Local variable
    };
    // For glb: length of name
    // For loc: number of allocs before active (0 = function args, 1 = start of function, etc...)
    unsigned int16_t val;
    
    // Number of times this variable is read from
    unsigned int16_t reads;
    // Number of times this variable is written to
    unsigned int16_t writes;
};

struct mccfunc
{
    int8_t * name;
    unsigned int8_t len;
    unsigned int8_t flag;
    
    struct mccvar * vartbl, ** vartail;
    
    struct mccfunc * next;
};
