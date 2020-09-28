		.text
mfsp_exit:	viemu_exit

trap_mfsp:	LDA 0, t_stack, 3	; Move Stack Pointer to SRC AC (actually DST AC)
		STA 0, @t_sac, 3

		JMP @mfsp_exit
