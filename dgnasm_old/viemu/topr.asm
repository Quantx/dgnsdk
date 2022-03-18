		.text
topr_exit:	viemu_exit

trap_topr:
#if TARGET_NOVA < 3
		LDA 0, @t_stack, 3		; Read top of stack into SRC AC (actually DST AC)
#else
		MFSP 2
		LDA 0, 0, 2
#endif
		STA 0, @t_sac, 3

		JMP @topr_exit
