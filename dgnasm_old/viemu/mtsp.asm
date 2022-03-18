		.text
mtsp_exit:	viemu_exit

trap_mtsp:	LDA 0, @t_sac, 3	; Move SRC AC to Stack Pointer
		STA 0, t_stack, 3

		JMP @mtsp_exit
