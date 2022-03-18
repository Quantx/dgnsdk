		.text
dsp_exit:	viemu_exit

trap_dsp:	DSZ t_stack, 3		; Decrement Stack Pointer
		MOV 0, 0

		JMP @dsp_exit
