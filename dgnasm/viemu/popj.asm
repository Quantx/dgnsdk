		.text
popj_exit:	viemu_exit

trap_popj:
#if TARGET_NOVA < 3
		LDA 0, @t_stack, 3		; Load value from stack

		DSZ 1, t_stack, 3               ; Decrement stack pointer
                MOV 1, 0
#else
		POPA 0				; Load value from stack
#endif
		STA 0, 046			; Store stack value into return address
		DSZ 046
		MOV 0, 0

		JMP @popj_exit
