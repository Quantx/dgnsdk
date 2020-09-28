		.text
pshr_exit:	viemu_exit

trap_pshr:	LDA 0, 046		; Get Current Address + 2
		INC 0, 0
		INC 0, 0

#define TARGET_NOVA < 3
		ISZ t_stack, 3		; Increment stack pointer
		MOV 0, 0

		STA 0, @t_stack, 3	; Write return address to stack
#else
		PSHA 0			; Write return address to stack
#endif
		JMP @pshr_exit
