		.text
popa_exit:	viemu_exit

trap_popa:	LDA 0, @t_stack, 3	; Read value off of stack
		STA 0, @t_sac, 3	; Store value into SRC AC (actually destination accumulator)

		DSZ @t_stack, 3		; Decrement stack
		MOV 0, 0

		JMP @popa_exit
