unsigned int16_t etop;
struct mcceval * estk[MAX_EXPR_NODE];

unsigned int16_t optr;
struct mccoper obuf[OPER_BUF_SIZE];


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
            ev->left = NULL;

            // Unary operators that strips l-value status
            if ( op == PostInc || op == PostDec || op == Inder || op == FnCall ) cn->right = ev, nd->type &= ~IR_LVAL;
            // Unary operator
            else if ( op == PreInc  || op == PreDec
            ||        op == Plus    || op == Minus
            ||        op == Deref
            ||        op == LogNot  || op == Not    ) cn->right = ev;
            // Binary operator
            else
            {
                if ( cn->left ) cn->right = ev;
                else
                {
                    // Binary operators that strips l-value status of the left operand
                    if ( op >= Ass && op <= OrAss
                    ||   op == Dot || op == FnCallArgs ) nd->type &= ~IR_LVAL;

                    cn->left = ev;
                    estk[etop++] = cn;
                    if ( etop > MAX_EXPR_NODE ) mccfail("expression stack full");
                }
            }
        }
    }

    struct mcceval * le = NULL; // Last node
    unsigned int8_t evlstk = zerosize; // Eval stack starts at end of zero page stack

    // https://leetcode.com/problems/binary-tree-postorder-traversal/discuss/45648/three-ways-of-iterative-postorder-traversing-easy-explanation
    ev = *estk;
    while ( ev || etop ) // Post order traversal
    {
        if ( ev ) estk[etop++] = ev, ev = ev->left;
        else
        {
            ev = estk[etop - 1];
            if ( !ev || ev == le )
            {
                etop--;

                // Increment the stack if both children are null, decrement if both are NOT null
//                evlstk += !nd->left + !nd->right - 1;

                // Get current instruction
                struct mccoper * oper = obuf + optr;

                if ( ev->right ) // Operator
                {
                    if ( ev->left ) // Binary operator
                    {

                    }
                }
                else // Operand
                {
                    unsigned int16_t size = ev->size = ev->st->size;

                    switch ( ev->st->oper )
                    {
                        case Variable: // Global var
                        case String: // Strings constants are effectively global vars
                            oper->op = OpAddrGlb;
                            oper->g.name = ev->st->name;
                            oper->g.len = ev->st->val;
                            oper->g.reg = evlstk;

                            break;
                        case VariableLocal: // Local var
                            oper->op = OpAddrLoc;
                            oper->l.mem = ev->st->val;
                            oper->l.reg = evlstk;

                            ev->addr = ev->st->val;

                            // Don't allocate any space on the eval stack for a zero page local var
                            if ( ev->st->val < zerosize && (ev->st->type & IR_LVAL) )
                            {
                                size = 0;
                            }

                            break;

                        case SmolNumber:
                            oper->op = OpValueByte;
                            oper->v.val = ev->st->val;
                            oper->v.reg = evlstk;
                            break;

                        case Number:
                            oper->op = OpValueWord;
                            oper->v.val = ev->st->val;
                            oper->v.reg = evlstk;
                            break;

                        case LongNumber:
                            oper->op = OpValueLong;
                            oper->v.valLong = ev->st->valLong;
                            oper->v.reg = evlstk;
                            break;
                    }

                    // Eval stack overran zero page
                    if ( evlstk + size > 0x100 ) mccfail("eval stack overflow");

                    evlstk += size;
                }

                optr++;

                // Do this last
                le = ev; ev = NULL;
            }
            else ev = ev->right;
        }
    }
}
