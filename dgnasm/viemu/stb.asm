		.text
stb_bytemask:	0xFF

trap_stb:	LDA 0, @7, 3		; Load contents of SRC AC into AC0 (AC3 still contains pointer to actbl)
		LDA 1, stb_bytemask
		AND 1, 0		; AC0 now contains byte to store

		LDA 2, @8, 3		; Load contents of DST AC into AC2
		MOVZR 2, 2		; Align byte pointer
		LDA 1, 0, 2		; Load word to store byte in into AC1
		MOV 2, 2, SNC		; Swap bytes if needed
		MOVS 1, 1
		LDA 2, stb_bytemask	; We loose the contents of DST AC here
		AND 2, 1
		ADD 0, 1		; AC1 now contains stored byte
		LDA 2, @8, 3		; Load contents of DST AC back into AC2
		MOVZR 2, 2, SNC		; Align byte pointer and skip switch if needed
		MOVS 1, 1
		STA 1, 0, 2		; Store word back into memory

                ; Reset CPU state
                LDA 0, 4, 3             ; Restore carry
                MOVR 0, 0

                LDA 0, 0, 3             ; Restore registers
                LDA 1, 1, 3
                LDA 2, 2, 3
                LDA 3, 3, 3

                ISZ 046                 ; Return to address after TRAP
                JMP @046
