		.text
mul_const:	-16
mul_ac3_ptr:	ac3

trap_mul:	LDA 0, t_ac0, 3		; Setup registers
		LDA 1, t_ac1, 3
		LDA 2, t_ac2, 3		; Load AC2
		LDA 3, mul_const

		; Taken from: How to use the Nova computers
mul_loop:	MOVR 1, 1, SNC
		MOVR 0, 0, SKP
		ADDZR 2, 0
		INC 3, 3, SZR
		JMP mul_loop
		MOVCR 1, 1
		; End of excerpt

		; Preform fast exit
		LDA 3, mul_ac3_ptr
		LDA 3, 1, 3		; Load carry bit
		MOVR 3, 3		; Restore carry bit

		LDA 3, mul_ac3_ptr	; Restore AC3

		ISZ 046
		JMP @046
