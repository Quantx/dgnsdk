#ifdef DEBUG
unsigned int16_t emitnum;

void printOper( struct mccoper * prop )
{
    // Output op name
    int8_t * opn = opNames[prop->op];
    int16_t opl = 0;
    
    while ( opn[opl] ) opl++;

    write( fd_dbg, opn, opl );
    write( fd_dbg, "\n", 1 );
    
    if ( prop->op >= OpEnd && prop->op <= OpReturn )
    {
        write( fd_dbg, "Statement: ", 11 );
        decwrite( fd_dbg, prop->s.id );
        write( fd_dbg, "\n", 1 );
    }
    else
    {
        // Output dest register
        write( fd_dbg, "Dest: ", 6 );
        decwrite( fd_dbg, prop->reg );
        write( fd_dbg, "|", 1 );
        decwrite( fd_dbg, prop->size );
        write( fd_dbg, "B\n", 2 );

        if ( prop->op >= OpValueByte && prop->op <= OpValueLong )
        {
            write( fd_dbg, "Val: ", 5 );
            if ( prop->op == OpValueLong ) octwrite( fd_dbg, prop->v.valLong );
            else decwrite( fd_dbg, prop->v.val );
            write( fd_dbg, "\n", 1 );
        }
        else if ( prop->op == OpAddrGlb )
        {
            write( fd_dbg, "Global Addr: ", 13 );
            write( fd_dbg, prop->a.name, prop->a.val );
            write( fd_dbg, "\n", 1 );
        }
        else if ( prop->op == OpAddrLoc )
        {
            write( fd_dbg, "Local Addr: ", 12 );
            decwrite( fd_dbg, prop->a.val );
            write( fd_dbg, "\n", 1 );
        }
        else
        {
            write( fd_dbg, "Comp L: ", 8 );
            decwrite( fd_dbg, prop->c.l_arg );
            write( fd_dbg, "|", 1 );
            decwrite( fd_dbg, prop->c.l_size );
            write( fd_dbg, "B\n", 2 );
            
            write( fd_dbg, "Comp R: ", 8 );
            decwrite( fd_dbg, prop->c.r_arg );
            write( fd_dbg, "|", 1 );
            decwrite( fd_dbg, prop->c.r_size );
            write( fd_dbg, "B\n", 2 );
        }
    }
    
    write( fd_dbg, "\n", 1 );
}

#endif

void emit( struct mccoper * emop )
{
    // Don't emit no-ops
    if ( emop->op == OpMov && emop->reg == emop->c.r_arg ) return;

    write( fd, emop, sizeof(struct mccoper CAST_NAME) );
#ifdef DEBUG
    printOper(emop);
#endif
}

void emitOpBuffer()
{
#if DEBUG
    write( fd_dbg, "*** Emit ", 9 );
    write(      2, "*** Emit ", 9 );
    
    decwrite( fd_dbg, emitnum );
    decwrite(      2, emitnum );
    
    emitnum++;
    
    write( fd_dbg, "\n", 1 );
    write(      2, "\n", 1 );
#endif
    struct mccoper * opout;
    for ( opout = obuf; opout != optr; opout++ )
    {
        emit( opout );
    }
    
    optr = obuf; // Reset optr
}

int16_t evlstk;
int16_t stkadj( int16_t bytes )
{
    // Conver to words
    if ( bytes < -1 || bytes > 1 ) bytes >>= 1;

    evlstk += bytes;

    if ( evlstk < curfunc->z_size ) mccfail("stack underflow");
    if ( evlstk >= MAX_ZEROPAGE   ) mccfail("stack overflow");
    
    if ( evlstk > curfunc->z_max ) curfunc->z_max = evlstk;
    
    return evlstk;
}

void generate(struct mcceval * cn)
{
    if ( !optr ) optr = obuf;
    
    regalloc(cn); // Spill and allocate the needed registers, returns how much of zero page was used to do this
    
    evlstk = curfunc->z_size; // Eval stak starts at the end of the zero page

    struct mcceval * ev, * ln = NULL; // Current and last nodes

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
            struct mccoper * r_op = ev->right->op;
        
            if ( ev->left )
            {
                struct mccoper * l_op = ev->left->op;
            
                if ( op >= Ass && op <= OrAss ) // Handle assignments
                {
                    if ( r_op->reg < curfunc->z_size ) // Right argument (left-hand-side of assignment) is currently allocated inside a register
                    {
                        if ( op == Ass )
                        {
                            // Alter previous MOV instruction to copy old value
                            l_op->reg = r_op->reg;
                            l_op->size = r_op->size;
                            
                            optr--; // Don't generate a new instruction (this should be harmless)
                        }
                        else
                        {
                            // Preform modification
                            optr->reg = r_op->reg;
                            optr->size = r_op->size;
                            
                            optr->c.l_arg = l_op->reg;
                            optr->c.l_size = l_op->size;
                            
                            optr->c.r_arg = r_op->reg;
                            optr->c.r_size = r_op->size;
                            
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
                        }
                    }
                    else // Right argument (left-hand-side of assignment) is currently allocated in main memory
                    {
                        if ( op == Ass )
                        {
                            optr--; // Last instruction should be a LOAD
#ifdef DEBUG_GEN
                            if ( optr->op != OpLoad ) mccfail( "Last op before assignment was not LOAD!" );
#endif
                            stkadj( -optr->size ); // Unallocate space from load op

                            optr->op = OpStore;

                            optr->reg = l_op->reg;
                            optr->size = ev->st->size;
                        }
                        else // Resultant type is always the same as the left-hand-side
                        {                            
                            // Store location of pointer
                            unsigned int8_t ptrreg = r_op->reg;

                            // Update last load instruction to keep the original pointer intact
                            stkadj( IR_PTR_SIZE );
                            r_op->reg += IR_PTR_SIZE >> 1;
#ifdef DEBUG_GEN
                            if ( l_op->size != ev->st->size ) mccfail("modify-assignment l_op->size != st->size");
#endif                               
                            // Move the result to the eval stack if needed
                            optr->reg = l_op->reg;
                            optr->size = l_op->size;
                            
                            optr->c.l_arg = l_op->reg;
                            optr->c.l_size = l_op->size;
                            
                            optr->c.r_arg = r_op->reg;
                            optr->c.r_size = r_op->size;
                    
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
                            optr->op = OpStore;
                            
                            optr->reg = l_op->reg;
                            optr->size = ev->st->size;
                            
                            optr->c.r_arg = r_op->reg;
                            optr->c.r_size = r_op->size;
                            
                            // Cleanup eval stack
                            stkadj( -(r_op->size + IR_PTR_SIZE) );
                        }
                    }
                }
                else
                {
                    if ( l_op->reg >= curfunc->z_size ) stkadj( -l_op->size );
                    if ( r_op->reg >= curfunc->z_size ) stkadj( -r_op->size );
                    
                    optr->reg = evlstk;
                    stkadj( optr->size = ev->st->size );
                    
                    optr->c.l_arg = l_op->reg;
                    optr->c.l_size = l_op->size;
                    
                    optr->c.r_arg = r_op->reg;
                    optr->c.r_size = r_op->size;
                    
                    switch ( op )
                    {
                    // *** Binary Operators ***
                        // case Ass: // Handled above
                        case Comma:
                            stkadj( -optr->size );
                            optr--; // Don't emit an operation
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
                            optr->op = OpAdd_P;
                            break;
                        case FnCallArgs:
                            optr->op = OpCall;
                            break;
                        case Arg:
                            optr->op = OpPush;
                            break;
#ifdef DEBUG_GEN
                        default: mccfail("no such binary operator");
#endif
                    }
                }
            }
            else
            {
                if ( r_op->reg >= curfunc->z_size ) stkadj( -r_op->size );
                
                // Move the result to the eval stack if needed
                optr->reg = evlstk;
                stkadj( optr->size = ev->st->size );
                
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
                        optr->op = ev->st->type & IR_LVAL ? OpPostIncLoad : OpPostInc;
                        break;
                    case PostDec:
                        optr->op = ev->st->type & IR_LVAL ? OpPostDecLoad : OpPostDec;
                        break;
                    case PreInc:
                        optr->op = ev->st->type & IR_LVAL ? OpPreIncLoad  : OpPreInc;
                        break;
                    case PreDec:
                        optr->op = ev->st->type & IR_LVAL ? OpPreDecLoad  : OpPreDec;
                        break;
                    case Minus:
                        optr->op = OpNegate;
                        break;
                    case LogNot:
                        optr->op = OpLogNot;
                        break;
                    case Not:
                        optr->op = OpNot;
                        break;
                    case Plus:
                        optr->op = OpMov; // Preform byte promotion to word
                        break;
                    case Deref: // Handled by l-value load
                    case Inder: // Does nothing
                        if ( r_op->reg >= curfunc->z_size ) stkadj( r_op->size - optr->size );
                        else stkadj( -optr->size );
                        optr--;
                        break;
#ifdef DEBUG_GEN
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
                    
                    // Last operation should be loading the function pointer, store a reference to that
                    optr->c.r_arg = (optr - 1)->reg;
                    optr->c.r_size = (optr - 1)->size;
                    break;

                case String: // Strings constant
                    optr->op = OpAddrGlb;
                    optr->a.name = ev->st->name;
                    optr->a.val = ev->st->val;

                    optr->reg = evlstk;
                    stkadj( optr->size = IR_PTR_SIZE );
                    break;

                case Variable: // Global var
                case VariableLocal: // Local var
#ifdef DEBUG_GEN
                    if ( !ev->var ) mccfail("non-existant variable durring code generation");
                    if ( (ev->var->flags & VAR_ALC_MASK) == VAR_ALC_NA ) mccfail("variable was not allocated anywhere durring code generation");
#endif
                    if ( (ev->var->flags & VAR_ALC_MASK) == VAR_ALC_ZP ) // Allocated in zero page
                    {
                        // Use a no-op MOV to pass along the address
                        // This MOV instruction can be repurpoused by an assignment
                        optr->op = OpMov;
                        optr->reg = ev->var->z_addr;
                        optr->size = ev->st->size;
                        
                        optr->c.r_arg = optr->reg;
                        optr->c.r_size = ev->var->size;
                        
                        ev->st->type &= ~IR_LVAL; // This is not an l-value
                    }
                    else // Allocated in main memory, we'll have to load it ourselves
                    {
                        if (ev->var->len) // Global variable
                        {
                            optr->op = OpAddrGlb;
                            optr->a.name = ev->st->name;
                        }
                        else // Local variable
                        {
                            optr->op = OpAddrLoc;
                        }

                        optr->a.val = ev->st->val;

                        optr->reg = evlstk;
                        
                        stkadj( optr->size = IR_PTR_SIZE );

                        // Mark complex types as non l-values as they cannot be dereferenced
                        unsigned int8_t type = ev->st->type & IR_TYPE_MASK;
                        
                        if ( type >= IR_STRUC && type <= IR_FUNC ) ev->st->type &= ~IR_LVAL;
                    }
                    break;

                case SmolNumber:
                    optr->op = OpValueByte;
                    optr->v.val = ev->st->val & 0xFF;
                    
                    optr->reg = evlstk;
                    
                    stkadj( optr->size = ev->st->size );
                    break;

                case Number:
                    optr->op = OpValueWord;
                    optr->v.val = ev->st->val;
                    
                    optr->reg = evlstk;
                    
                    stkadj( optr->size = ev->st->size );
                    break;

                case LongNumber:
                    optr->op = OpValueLong;
                    optr->v.valLong = ev->st->valLong;
                    
                    optr->reg = evlstk;
                    
                    stkadj( optr->size = ev->st->size );
                    break;
#ifdef DEBUG_GEN
                default: mccfail("no such operand");
#endif
            }
        }

        // Inderection (Address-of) strips l-value
        if ( pv && pv->st->oper == Inder ) ev->st->type &= ~IR_LVAL;

        // Preform l-to-r value conversion
        if ( ev->st->type & IR_LVAL )
        {
            optr++;
            optr->op = OpLoad;
            
            optr->c.r_arg = optr->reg = (optr-1)->reg;
            optr->c.r_size = optr->size = ev->st->size; // This should be the actual size in bytes
            
            if ( optr->reg < curfunc->z_size ) // Can't load into zero page, allocate room on the stack
            {
                optr->reg = evlstk;
                stkadj( optr->size );
            }
            else
            {
                unsigned int8_t sz = optr->size;
                if ( sz < 2 ) sz = 2;
                
                // Update eval stack size with loaded type's size
                stkadj( sz - IR_PTR_SIZE );
            }
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
        
        if ( pv )
        {
            if ( pv->left == ev )
            {
                // Discard result if left child of comma operator
                if ( pv->st->oper == Comma ) stkadj( -ev->st->size );
                else if ( pv->st->oper > Ass && pv->st->oper <= OrAss ) // Ensure right-hand-side of modify-assignment has correct size and is on eval stack
                {
                    optr->size = pv->st->size;
                    if (!optr->size) optr->size++;
                    
                    if ( optr->reg < curfunc->z_size )
                    {
                        optr->reg = evlstk;
                        stkadj( optr->size );
                    }
                }
            }
        }
        
#ifdef DEBUG
        if ( optr->op == OpValueByte && optr->v.val > 0xFF ) mccfail( "OpValueByte had value greater than 0xFF" );
#endif

        // Increment current operation pointer
        ev->op = optr++;
    }
}
