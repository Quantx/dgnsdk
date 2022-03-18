		.text
or_exit:	viemu_exit
trap_or:	LDA 0, @t_sac, 3	; Load contents of SRC AC into AC0 (AC3 still contains pointer to actbl)
		LDA 1, @t_dac, 3	; Load contents of DST AC into AC1

		; P|Q = !( !P & !Q )
		COM 0, 0
		COM 1, 1
		AND 1, 0
		COM 0, 0

		STA 0, @t_dac, 3	; Store result back into DST AC

		JMP @or_exit
