#include "../../lib/unistd.h"
#include "../mcc/tokens.h"
#include "../mcc/statements.h"

#define MAX_STATEMENT 64
#define MAX_EVAL_STK 64 // Maximum size (in words) of the zero page expression evaluation stack
#define MAX_OPER_BUF 1024 // Maximum number of instructions per statement

#include "../mcc/opcodes.h"

// Compiler fail function
void mccfail(int8_t * msg);

// Used to pair expression nodes with their corresponding opcodes
struct mcceval
{
    // Address and size of register containing the final value of this node
    unsigned int8_t addr;
    unsigned int8_t size;

    struct mccstmt * st;
    struct mcceval * left, * right;
};
