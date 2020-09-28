		.text
ldb_bytemask:	0xFF
ldb_exit:	viemu_exit

trap_ldb:	LDA 2, @t_sac, 3	; Load contents of SRC AC (AC3 still holds pointer to actbl)
		MOVZR 2, 2		; Align address
		LDA 0, 0, 2		; Load the word containing our byte into AC0
		MOV 2, 2, SNC		; Swap bytes if needed
		MOVS 0, 0
		LDA 1, ldb_bytemask
		AND 1, 0		; AC0 now contains just our byte
		STA 0, @t_dac, 3	; Store our byte in DST AC

		JMP @ldb_exit
