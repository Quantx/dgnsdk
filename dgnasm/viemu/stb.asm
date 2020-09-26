		.text
stb_dstsrc:	dstsrc
stb_ac_mask:	3
stb_actbl:	ac0
stb_bytemask:	0xFF

trap_stb:	LDA 0, @stb_dstsrc

		LDA 1, stb_ac_mask      ; Compute SRC AC
                AND 0, 1
                LDA 3, stb_actbl
                ADD 1, 3

		LDA 3, 0, 3		; Load contents of SRC AC
		LDA 1, stb_bytemask
		AND 3, 1		; AC1 now contains byte to store

		LDA 2, stb_ac_mask	; Compute DST AC
		MOVS 0, 0
		AND 0, 2
		LDA 3, stb_actbl
		ADD 2, 3

		LDA 3, 0, 3		; Load contents of DST AC
		MOVZR 3, 3		; Align byte pointer
		LDA 0, 0, 3		; Load word to store byte in into AC0
		MOV 3, 3, SNC		; Swap bytes if needed
		MOVS 0, 0
		LDA 2, stb_bytemask
		AND 2, 0
		ADD 1, 0		; AC0 now contains stored byte
		MOV 3, 3, SNC		; Carry must be the same as before
		MOVS 0, 0
		STA 0, 0, 3		; Store word back into memory

                ; Reset CPU state
                LDA 3, stb_actbl
                LDA 0, 4, 3             ; Restore carry
                MOVR 0, 0

                LDA 0, 0, 3             ; Restore registers
                LDA 1, 1, 3
                LDA 2, 2, 3
                LDA 3, 3, 3

                ISZ 046                 ; Return to address after TRAP
                JMP 046
