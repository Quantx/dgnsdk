#include "../../lib/unistd.h"
#include "../mcc/tokens.h"
#include "../mcc/statements.h"

#ifdef DEBUG
#define DEBUG_NODE 1
#define DEBUG_ALLOC 1
#define DEBUG_GEN 1
#endif

#define MAX_STATEMENT 64
#define MAX_EVAL_STK 64 // Maximum size (in words) of the zero page expression evaluation stack
#define MAX_OPER_BUF 256 // Maximum number of instructions per statement
#define MAX_STATEMENT_STK 64 // Maximum depth of statement stack depth

#define MAX_ANALYSIS_BRK 4096 // Number of bytes for analysis brk region

// 216 = 256 (size of zero page) - 40 (number in use by system: 0 through 047)
#define MAX_REG 32 // Number of registers available for allocation per function
#define MAX_ZEROPAGE 216 // The absolute maximum number of registers available for allocation (this can change based on the OS and memory mapping)

#include "../mcc/opcodes.h"

// Compiler fail function
void mccfail(int8_t * msg);

// Used to pair expression nodes with their corresponding opcodes
struct mcceval
{
    struct mccoper * op;
    struct mccstmt * st;
    struct mccvar * var;
    struct mcceval * left, * right, * parent;
};

// Used to store information about flow control statements
struct mccfcst
{
    struct mccstmt * st;
    struct mcceval * ev;
    unsigned int16_t val;
};

#define VAR_ALC_ZP 0 // Allocated in main memory with a pointer to it in zero page
#define VAR_ALC_MM 1 // Only allocated in main memory
#define VAR_ALC_DA 2 // Don't allocate (used for arrays, structs, and functions), also used when the address of a variable is taken
#define VAR_ALC_NA 3 // Not allocated anywhere
#define VAR_ALC_MASK 0b11
#define VAR_ALC_REG_VALID 4 // This is set when the mccvar.reg field is valid. This can remain set even after the var is unallocated.

struct mccvar
{
    union
    {
        int8_t * name; // Global variable
        unsigned int16_t addr; // Local variable address (assuming everything is allocated to the stack)
    };
    // Length of global var's name, 0 for local var
    unsigned int16_t len;

    unsigned int8_t flags;
    unsigned int8_t reg; // Zero page address
    
//    unsigned int16_t s_addr; // Spill address
    unsigned int16_t size; // Size in bytes of this variable
    
    unsigned int16_t lac; // Last access count
    int16_t stmt; // The top most element of the statement stack at time of allocation
    
    struct mccvar * next;
};

struct mccfunc
{
    int8_t * name;
    unsigned int8_t len;
    unsigned int8_t z_size; // Number of registers currently in use
    
    unsigned int16_t s_size; // Stack size
    
    unsigned int16_t vac; // Variable access count
    
    struct mccvar * vartbl, ** vartail;
    
    struct mccfunc * next;
};
