unsigned int16_t etop;
struct mcceval * estk[MAX_EXPR_NODE];

// Deserialize and reconstruct expression tree
void expr(struct mccstmt * nd)
{
     if ( nd && nd->oper == Void ) nd = NULL; // Nothing to do here, this is a void expression

     struct mcceval * ev = sbrk(sizeof(struct mcceval CAST_NAME));
     if ( ev == SBRKFAIL ) mccfail( "unable to allocate room for eval node" );

     ev->st = nd;
     ev->left = ev->right = NULL;

     etop = 0;
     while ( ev || etop ) // Preorder
     {
         struct mcceval * cn = ev ? ev : estk[--etop];

         // Leaf node
         if ( cn->st->oper >= VariableLocal && cn->st->oper <= DblNumber ) nd = NULL;
         // Operators
         else
         {
             if ( !(nd = node()) ) ircfail("unexpected end of expression");

             ev = sbrk(sizeof(struct mcceval CAST_NAME));
             if ( ev == SBRKFAIL ) mccfail( "unable to allocate room for eval node" );

             ev->st = nd;
             ev->left = ev->right = NULL;

             // Inderection removes lvalue status
             if ( nd->oper == Deref ) cn->right = ev, nd->type &= ~IR_LVAL;
             // Unary operator
             else if ( cn->oper == PostInc || cn->oper == PostDec
             ||        cn->oper == PreInc  || cn->oper == PreDec
             ||        cn->oper == Plus    || cn->oper == Minus
             ||        cn->oper == Defer
             ||        cn->oper == LogNot  || cn->oper == Not
             ||        cn->oper == Sizeof
             ||        cn->oper == FnCall                         ) cn->right = ev;
             // Operator
             else
             {
                 if ( cn->left ) cn->right = nd;
                 else
                 {
                     cn->left = nd;
                     estk[etop++] = cn;
                     if ( etop > MAX_EXPR_NODE ) ircfail("expression stack full");
                 }
             }
         }
     }
}

void emit( struct mccstmt * nd )
{
    struct mccstmt * ln = NULL; // Last node
    unsigned int8_t evlstk = 0;

    etop = zerosize; // Eval stack starts at end of zero page stack
    // https://leetcode.com/problems/binary-tree-postorder-traversal/discuss/45648/three-ways-of-iterative-postorder-traversing-easy-explanation
    while ( nd || etop ) // Post order traversal (depth first)
    {
        if ( nd ) estk[etop++] = nd, nd = nd->left;
        else
        {
            nd = estk[etop - 1];
            if ( !nd || nd == ln )
            {
                etop--;

                // Increment the stack if both children are null, decrement if both are NOT null
//                evlstk += !nd->left + !nd->right - 1;

                if ( nd->right ) // Operator
                {
                    unsigned int8_t 

                    if ( nd->left ) // Binary operator
                    {

                    }
                }
                else // Operand
                {
                    unsigned int16_t size = nd->size;

                    switch ( nd->oper )
                    {
                        case Variable: // Global var

                            break;
                        case VariableLocal: // Local var

                            // Don't allocate any space on the eval stack for a zero page local var
                            if ( nd->val < zerosize ) size = 0;
                            break;
                    }

                    // Eval stack overran zero page
                    if ( evlstk + size > 0x100 ) mccfail("eval stack overflow");

                    evlstk += nd->size;
                }

                // Do this last
                nd = ln; nd = NULL;
            }
            else nd = nd->right;
        }
    }
}
