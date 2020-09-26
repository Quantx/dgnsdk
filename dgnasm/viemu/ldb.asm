		.text
ldb_dstsrc:	dstsrc
ldb_ac_mask:	3
ldb_actbl:	ac0
ldb_bytemask:	0xFF

trap_ldb:	LDA 0, @ldb_dstsrc

		LDA 1, ldb_ac_mask	; Compute SRC AC
		AND 0, 1
		LDA 3, ldb_actbl
		ADD 1, 3

		LDA 3, 0, 3		; Load contents of SRC AC
		MOVZR 3, 3		; Align address
		LDA 2, 0, 3		; Load the word containing our byte
		MOV 3, 3, SNC		; Swap bytes if needed
		MOVS 2, 2
		LDA 3, ldb_bytemask	; Get only this byte
		AND 3, 2

		LDA 1, ldb_ac_mask	; Compute DST AC
		MOVS 0, 0
		AND 0, 1
		LDA 3, ldb_actbl
		ADD 1, 3

		STA 2, 0, 3		; Put byte back into accumulator

		; Reset CPU state
		LDA 3, ldb_actbl
		LDA 0, 4, 3		; Restore carry
		MOVR 0, 0

		LDA 0, 0, 3		; Restore registers
		LDA 1, 1, 3
		LDA 2, 2, 3
		LDA 3, 3, 3

		ISZ 046			; Return to address after TRAP
		JMP 046

