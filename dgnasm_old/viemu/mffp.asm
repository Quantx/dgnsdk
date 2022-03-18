		.text
mffp_exit:	viemu_exit

trap_mffp:	LDA 0, t_frame, 3	; Move from Frame Pointer to SRC AC (actually DST AC)
		STA 0, @t_sac, 3

		JMP @mffp_exit
