unsigned int16_t etop;
struct mcceval * estk[MAX_EXPR_NODE];

struct mcceval * expr(struct mccstmt * nd)
{
    if ( nd && nd->oper == Void ) nd = NULL; // Nothing to do here, this is a void expression

    // Deserialize and reconstruct expression tree
    struct mcceval * ev = sbrk(sizeof(struct mcceval CAST_NAME));
    if ( ev == SBRKFAIL ) mccfail( "unable to allocate room for eval node" );

    ev->st = nd;
    ev->left = ev->right = ev->parent = NULL;

    struct mcceval * root = ev; // Record root node

    etop = 0;
    while ( ev || etop ) // Preorder
    {
        struct mcceval * cn = ev ? ev : estk[--etop];
        unsigned int8_t op = cn->st->oper;

        // Leaf node
        if ( op >= VariableLocal && op <= DblNumber ) ev = NULL;
        // Operators
        else
        {
            if ( !(nd = node()) ) mccfail("unexpected end of expression");

            ev = sbrk(sizeof(struct mcceval CAST_NAME));
            if ( ev == SBRKFAIL ) mccfail( "unable to allocate room for eval node" );

            ev->st = nd;
            ev->parent = cn;
            ev->left = ev->right = NULL;

            // Unary operators that strips l-value status
            if ( op == PreInc  || op == PreDec
            ||   op == PostInc || op == PostDec
            ||   op == Inder
            ||   op == FnCall                     ) cn->right = ev, nd->type &= ~IR_LVAL;
            // Unary operator
            else if ( op == Minus   || op == Plus
            ||        op == Deref
            ||        op == LogNot  || op == Not  ) cn->right = ev;
            // Binary operator
            else
            {
                if ( cn->left ) // Left side done, handle right side now
                {
                    // Ensure left hand side of Addition operator is always the pointer
                    // Swap the left and right nodes of assignment operators
                    if ( (op >= Ass && op <= OrAss)
                    ||   (op == Add && (nd->type & IR_TYPE_MASK) == IR_PTR) )
                    {
                        // Swap pointer to left hand side
                        cn->right = cn->left;
                        cn->left = ev;
                    }
                    else cn->right = ev;
                }
                else // Do left side first
                {
                    // Binary operators that strips l-value status of the left operand
                    if ( op >= Ass && op <= OrAss
                    ||   op == Dot || op == FnCallArgs ) nd->type &= ~IR_LVAL;

                    cn->left = ev;
                    estk[etop++] = cn;
                    if ( etop > MAX_EXPR_NODE )
                    {
#ifdef DEBUG
                        decwrite(2, etop);
                        write(2, "\n", 1);
#endif
                        mccfail("expression stack full");
                    }
                }
            }
        }
    }
    
#ifdef DEBUG
    write( 2, "Ingest done\n", 12 );
#endif

    return root;
}
