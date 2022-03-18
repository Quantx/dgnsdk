		.text
div_const:	-16
div_ac3_ptr:	ac3

trap_div:	LDA 0, t_ac0, 3		; Load AC0
		LDA 2, t_ac2, 3		; Load AC2

		; Taken from: How to use the Nova computers
		SUBZ# 2, 0, SZC         ; Test for overflow
                JMP div_finish

		LDA 1, t_ac1, 3		; Setup rest of accumulators
		LDA 3, div_const

		MOVZL 1, 1

div_loop:	MOVL 0, 0
		SUB# 2, 0, SZC
		SUB 2, 0
		MOVL 1, 1
		INC 3, 3, SZR
		JMP div_loop
		SUBO 3, 3, SKP
div_finish:	SUBZ 3, 3

		; Preform a fast exit
		LDA 3, div_ac3_ptr
                LDA 3, 1, 3		; Load carry bit
		MOVR 3, 3		; Restore carry

		LDA 3, div_ac3_ptr	; Restore AC3

		ISZ 046
		JMP @046
