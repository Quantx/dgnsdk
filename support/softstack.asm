; Software stack push routine
DGNMCC_PUSH_SUB:        STA 3, DGNMCC_PUSH_RET  ; Store return address
                        LDA 3, DGNMCC_STACK_PTR ; Load stack pointer
                        STA 0, 0, 3             ; Push main accumulator to stack
                        INC 3, 3                ; Increment stack pointer AFTER writing
                        STA 3, DGNMCC_STACK_PTR ; Write back stack pointer
                        JMP @DGNMCC_PUSH_RET    ; Return from function
DGNMCC_PUSH_RET:        0
