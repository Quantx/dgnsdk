		.text
topw_exit:	viemu_exit

trap_topw:	LDA 0, @t_sac, 3		; Write SRC AC into top of stack
#if TARGET_NOVA < 3
		STA 0, @t_stack, 3
#else
		MFSP 2
		STA 0, 0, 2
#endif
		JMP @topw_exit
