int16_t opClass( struct mccstmt * st )
{
    unsigned int8_t t = st->type & IR_TYPE_MASK;
    switch (t)
    {
        case IR_VOID:
            return OP_CLASS_VOID;
        case IR_CHR:
        case IR_INT:
        case IR_LNG:
            return OP_CLASS_SIGNED;
        case IR_UCHR:
        case IR_UINT:
        case IR_ULNG:
            return OP_CLASS_UNSIGNED;
        case IR_FPV:
        case IR_DBL:
            return OP_CLASS_FLOAT;
        case IR_PTR:
        case IR_ARRAY: // Remember, arrays aren't always pointers, try reconsidering this later
            return OP_CLASS_POINTER;
    }
    
    return -1;
}

void analyze(struct mcceval * cn)
{
    int16_t evlstk = zerosize; // Eval stack starts at end of zero page stack
    struct mcceval * ev, * ln = NULL; // Current and last nodes

    // https://leetcode.com/problems/binary-tree-postorder-traversal/discuss/45648/three-ways-of-iterative-postorder-traversing-easy-explanation
    while ( cn || etop ) // Post order traversal
    {
        if ( cn )
        {
            estk[etop++] = cn;
            cn = cn->left;
            continue;
        }
        
        ev = estk[etop - 1];
        
        if ( ev->right && ln != ev->right )
        {
            cn = ev->right;
            continue;
        }
        
        ln = ev; etop--;

        struct mcceval * pv = ev->parent;

        unsigned int8_t op = ev->st->oper;
        unsigned int8_t opc = opClass(ev->st);
    }
}
