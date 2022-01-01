#define MAX_EXPR_NODE 256  // Maximum size of the expression node stack

#define IR_PTR_SIZE 2 // Size of a pointer in bytes

// Primative types
#define IR_VOID 0 // ( 0 bits) Void
#define IR_CHR  1 // ( 8 bits) Signed Character
#define IR_UCHR 2 // ( 8 bits) Unsigned Character
#define IR_INT  3 // (16 bits) Signed Integer
#define IR_UINT 4 // (16 bits) Unsigned Integer
#define IR_LNG  5 // (32 bits) Signed Long
#define IR_ULNG 6 // (32 bits) Unsigned Long
#define IR_FPV  7 // (32 bits) DG Nova Float
#define IR_DBL  8 // (64 bits) DG Nova Double
// Complex types
#define IR_STRUC 9
// Derived types
#define IR_ARRAY 10
#define IR_FUNC  11
#define IR_PTR   12
// Types mask
#define IR_TYPE_MASK 0b1111

#define IR_LVAL (1 << 4) // Flag is set if this node is an L-value

// Intermediate Represntation Compiler Block
struct mccstmt
{
    unsigned int8_t oper;
    unsigned int8_t type;
    unsigned int16_t size; // Size of the type in bytes
    union {
        struct {
            unsigned int16_t val;
            int8_t * name;
        };
        unsigned int32_t valLong;
    };
};
