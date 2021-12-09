struct mccoper * optr;
struct mccoper obuf[MAX_OPER_BUF];

void emit( struct mccoper * emop )
{
    write( fd, emop, sizeof(struct mccoper CAST_NAME) );
    
#ifdef DEBUG
    // Output op name
    int8_t * opn = opNames[emop->op];
    int16_t opl = 0;
    
    while ( opn[opl] ) opl++;

    write( fd_dbg, opn, opl );
    
    // Output dest register
    write( fd_dbg, "\nDest: ", 7 );
    decwrite( fd_dbg, emop->reg );
    write( fd_dbg, "|", 1 );
    decwrite( fd_dbg, emop->size );
    write( fd_dbg, "\n", 1 );

    if ( emop->op >= OpValueByte && emop->op <= OpValueLong )
    {
        write( fd_dbg, "Val: ", 5 );
        if ( emop->op == OpValueLong ) octwrite( fd_dbg, emop->v.valLong );
        else decwrite( fd_dbg, emop->v.val );
        write( fd_dbg, "\n", 1 );
    }
    else if ( emop->op == OpAddrGlb )
    {
        write( fd_dbg, "Global Addr: ", 13 );
        write( fd_dbg, emop->a.name, emop->a.val );
        write( fd_dbg, "\n", 1 );
    }
    else if ( emop->op == OpAddrLoc )
    {
        write( fd_dbg, "Local Addr: ", 12 );
        decwrite( fd_dbg, emop->a.val );
        write( fd_dbg, "\n", 1 );
    }
    else if ( emop->op >= OpEnd && emop->op <= OpReturn )
    {
        write( fd_dbg, "Statement: ", 12 );
        decwrite( fd_dbg, emop->s.id );
        write( fd_dbg, "\n", 1 );
    }
    else
    {
        write( fd_dbg, "Comp L: ", 8 );
        decwrite( fd_dbg, emop->c.l_arg );
        write( fd_dbg, "|", 1 );
        decwrite( fd_dbg, emop->c.l_size );
        write( fd_dbg, "\n", 1 );
        
        write( fd_dbg, "Comp R: ", 8 );
        decwrite( fd_dbg, emop->c.r_arg );
        write( fd_dbg, "|", 1 );
        decwrite( fd_dbg, emop->c.r_size );
        write( fd_dbg, "\n", 1 );
    }
#endif
}

void emitOpBuffer()
{
    struct mccoper * opout;
    for ( opout = obuf; opout != optr; opout++ )
    {
        emit( opout );
    }
    
    optr = NULL;
}

void generate(struct mcceval * cn)
{
    if ( !optr ) optr = obuf;

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

        if ( ev->right )
        {
            if ( ev->left )
            {
                struct mccoper * l_op = ev->left->op;
                struct mccoper * r_op = ev->right->op;
                
                unsigned int8_t scratch = evlstk;
                
                evlstk -= l_op->size + r_op->size;
                
                // Move the result to the evlstk if needed
                optr->reg = l_op->reg < zerosize ? evlstk : l_op->reg;
                optr->size = ev->st->size;
                
                evlstk += ev->st->size;
                
                optr->c.l_arg = l_op->reg;
                optr->c.l_size = l_op->size;
                
                optr->c.r_arg = r_op->reg;
                optr->c.r_size = r_op->size;
                
                if ( op > Ass && op <= OrAss ) // Handle read-modify-write assignments (no
                {
                    // Push existing value onto eval stack
                    optr->op = OpLoad;
                    optr->reg = scratch;
                    optr++;
                    
                    // Preform modification
                    optr->reg = l_op->reg;
                    optr->size = ev->st->size;
                    
                    optr->c.l_arg = l_op->reg;
                    optr->c.l_size = l_op->size;
                    
                    optr->c.r_arg = scratch;
                    optr->c.r_size = optr->size;
                    
                    switch ( op )
                    {
                        case AddAss:
                            optr->op = opc == OP_CLASS_POINTER ? OpAdd_P : OpAdd;
                            break;
                        case SubAss:
                            optr->op = opc == OP_CLASS_POINTER ? OpSub_P : OpSub;
                            break;
                        case Mul:
                            optr->op = opc == OP_CLASS_SIGNED ? OpMul_S : OpMul_U;
                            break;
                        case Div:
                            optr->op = opc == OP_CLASS_SIGNED ? OpDiv_S : OpDiv_U;
                            break;
                        case Mod:
                            optr->op = opc == OP_CLASS_SIGNED ? OpMod_S : OpMod_S;
                            break;
                        case ShlAss:
                            optr->op = OpShl;
                            break;
                        case ShrAss:
                            optr->op = opc == OP_CLASS_SIGNED ? OpShr_S : OpShr_U;
                            break;
                        case AndAss:
                            optr->op = OpAnd;
                            break;
                        case XorAss:
                            optr->op = OpXor;
                            break;
                        case OrAss:
                            optr->op = OpOr;
                            break;
                    }
                    optr++;
                    
                    // Store final result
                    optr->reg = l_op->reg;
                    optr->size = ev->st->size;
                    
                    optr->c.r_arg = r_op->reg;
                    optr->c.r_size = r_op->size;
                    
                    optr->op = OpStore;
                }
                else switch ( op )
                {
                // *** Binary Operators ***
                    // case Comma: // Handled by left child
                    case Ass:
                        optr->op = OpStore;
                        break;
                    case LogOr:
                        optr->op = OpLogOr;
                        break;
                    case LogAnd:
                        optr->op = OpLogAnd;
                        break;
                    case Or:
                        optr->op = OpOr;
                        break;
                    case Xor:
                        optr->op = OpXor;
                        break;
                    case And:
                        optr->op = OpAnd;
                        break;
                    case Eq:
                        optr->op = OpEq;
                        break;
                    case Neq:
                        optr->op = OpNeq;
                        break;
                    case Less:
                        optr->op = opc == OP_CLASS_SIGNED ? OpLess_S : OpLess_U;
                        break;
                    case LessEq:
                        optr->op = opc == OP_CLASS_SIGNED ? OpLessEq_S : OpLessEq_U;
                        break;
                    case Great:
                        optr->op = opc == OP_CLASS_SIGNED ? OpGreat_S : OpGreat_U;
                        break;
                    case GreatEq:
                        optr->op = opc == OP_CLASS_SIGNED ? OpGreatEq_S : OpGreatEq_U;
                        break;
                    case Shl:
                        optr->op = OpShl;
                        break;
                    case Shr:
                        optr->op = opc == OP_CLASS_SIGNED ? OpShr_S : OpShr_U;
                        break;
                    case Add:
                        optr->op = opc == OP_CLASS_POINTER ? OpAdd_P : OpAdd;
                        break;
                    case Sub:
                        optr->op = opc == OP_CLASS_POINTER ? OpSub_P : OpSub;
                        // Subtracting a pointer from a pointer
                        if ( opClass(ev->right->st) == OP_CLASS_POINTER ) optr->op = OpSub_PP;
                        break;
                    case Mul:
                        optr->op = opc == OP_CLASS_SIGNED ? OpMul_S : OpMul_U;
                        break;
                    case Div:
                        optr->op = opc == OP_CLASS_SIGNED ? OpDiv_S : OpDiv_U;
                        break;
                    case Mod:
                        optr->op = opc == OP_CLASS_SIGNED ? OpMod_S : OpMod_S;
                        break;
                    case Arrow:
                        optr->op = OpArrow;
                        break;
                    case Dot:
                        optr->op = OpDot;
                        break;
                    case Square_L:
                        optr->op = OpArray;
                        break;
                    case FnCallArgs:
                        optr->op = OpCall;
                        break;
                    case Arg:
                        optr->op = OpPush;
                        break;
#ifdef DEBUG
                    default: mccfail("no such binary operator");
#endif
                }
            }
            else
            {
                struct mccoper * r_op = ev->right->op;

                evlstk -= r_op->size;
                
                // Move the result to the evlstk if needed
                optr->reg = r_op->reg < zerosize ? evlstk : r_op->reg;
                optr->size = ev->st->size;
                
                evlstk += ev->st->size;
                
                optr->c.r_arg = r_op->reg;
                optr->c.r_size = r_op->size;
                 
                switch ( op )
                {
                // *** Unary Operators ***
                    case FnCall:
                        optr->op = OpCall;
                        optr[1] = *optr;
                        
                        optr->op = OpArg;
                        optr++;                    

                        break;
                    case PostInc:
                        optr->op = OpPostInc;
                        break;
                    case PostDec:
                        optr->op = OpPostDec;
                        break;
                    case PreInc:
                        optr->op = OpPreInc;
                        break;
                    case PreDec:
                        optr->op = OpPreDec;
                        break;
                  //case Plus: // Does nothing
                    case Minus:
                        optr->op = OpNegate;
                        break;
                    case Deref:
                        optr->op = OpLoad;
                        break;
                  //case Inder: // Does nothing
                    case LogNot:
                        optr->op = OpLogNot;
                        break;
                    case Not:
                        optr->op = OpNot;
                        break;
#ifdef DEBUG
                    default: mccfail("no such unary operator");
#endif
                }
            }
        }
        else // *** Operands ***
        {
            switch ( op )
            {
                case StartOfArgs: // Not an operand, just tells us that we're about to start pushing arguments for a function call
                    optr->op = OpArg;
                    break;
            
                case Variable: // Global var
                case String: // Strings constants are effectively global vars
                    optr->op = OpAddrGlb;
                    optr->a.name = ev->st->name;
                    optr->a.val = ev->st->val;

                    optr->size = IR_PTR_SIZE;
                    optr->reg = evlstk;
                    
                    evlstk += IR_PTR_SIZE;
                    break;

                case VariableLocal: // Local var
                    optr->op = OpAddrLoc;
                    optr->a.val = ev->st->val;

                    optr->size = IR_PTR_SIZE;

                    // Don't allocate any space on the eval stack for a zero page local var which is also an l-value
                    if ( ev->st->val < zerosize && (ev->st->type & IR_LVAL) )
                    {
                        optr->reg = ev->st->val;
                    }
                    else
                    {
                        optr->reg = evlstk;
                        
                        evlstk += IR_PTR_SIZE;
                    }

                    break;

                case SmolNumber:
                    optr->op = OpValueByte;
                    optr->v.val = ev->st->val;
                    
                    optr->size = ev->st->size;
                    optr->reg = evlstk;
                    
                    evlstk += ev->st->size;
                    break;

                case Number:
                    optr->op = OpValueWord;
                    optr->v.val = ev->st->val;
                    
                    optr->size = ev->st->size;
                    optr->reg = evlstk;
                    
                    evlstk += ev->st->size;
                    break;

                case LongNumber:
                    optr->op = OpValueLong;
                    optr->v.valLong = ev->st->valLong;
                    
                    optr->size = ev->st->size;
                    optr->reg = evlstk;
                    
                    evlstk += ev->st->size;
                    break;
#ifdef DEBUG
                default: mccfail("no such operand");
#endif
            }
        }

        // Preform l-to-r value conversion for non-zero-page registers
        if ( ev->st->val >= zerosize && (ev->st->type & IR_LVAL) )
        {
            optr++;
            optr->op = OpLoad;
            
            optr->c.r_arg = optr->reg = (optr-1)->reg;
            optr->c.r_size = (optr-1)->size;
            
            optr->size = ev->st->size;
            
            // Update eval stack size with loaded type's size
            evlstk += ev->st->size - optr->c.r_size;
        }
        
        /* Handle implicit conversion between floats & ints which happens durring:
        
            Casting (Obviously)
            Assignment
            Function Arguments
            Return statements
        
            Binary arithmetic: *, /, %, +, -
            Relational operators: <, >, <=, >=, ==, !=
            Binary bitwise arithmetic: &, ^, |
            The ternary conditional operator: ?
        
            !!!TODO!!!
        */
        
        // Discard result if left child of comma operator
        if ( pv && pv->st->oper == Comma && pv->left == ev ) evlstk -= ev->st->size;

        // Handle case where the parent node is an assignment expression to a variable in zeropage
        // TODO

        // Eval stack overran zero page
        if ( evlstk > 0x100    ) mccfail("eval stack overflow");
        if ( evlstk < zerosize ) mccfail("eval stack underflow");

        // Increment current operation pointer
        ev->op = optr++;
    }
}
