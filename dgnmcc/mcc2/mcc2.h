#include "../../lib/unistd.h"
#include "../mcc/tokens.h"
#include "../mcc/statements.h"

#define MAX_STATEMENT 64
#define MAX_EVAL_STK 64 // Maximum size (in words) of the zero page expression evaluation stack

#include "../mcc/opcodes.h"

// Compiler fail function
void mccfail(int8_t * msg);

// Used to pair expression nodes with their corresponding opcodes
struct mcceval
{
    struct mccstmt * st;
    struct mccoper op;
    struct mcceval * left, * right;
};
