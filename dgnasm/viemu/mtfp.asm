		.text
mtfp_exit:	viemu_exit

trap_mtfp:	LDA 0, @t_sac, 3	; Move SRC AC to Frame Pointer
		STA 0, t_frame, 3

		JMP @mtfp_exit
