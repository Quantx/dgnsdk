		.text
psha_exit:	viemu_exit

trap_psha:	ISZ t_stack, 3		; Increment stack pointer
		MOV 0, 0

		LDA 0, @t_sac, 3	; Load value from SRC AC into AC0 (AC3 still contains pointer to ac table)
		STA 0, @t_stack, 3	; Write word onto stack

		JMP @psha_exit
