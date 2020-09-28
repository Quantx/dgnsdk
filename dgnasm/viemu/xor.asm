		.text
xor_exit:	viemu_exit

trap_xor:	LDA 0, @t_sac, 3	; Load contents of SRC AC into AC0 (AC3 still contains pointer to actbl)
		LDA 1, @t_dac, 3	; Load contents of DST AC into AC1

		; P ^ Q = ( !(P & Q) ) & ( !( !P & !Q ) )
		MOV 0, 2
		AND 1, 2
		COM 2, 2

		COM 0, 0
		COM 1, 1
		AND 1, 0
		COM 0, 0

		AND 2, 0

		STA 0, @t_dac, 3	; Store result back into DST AC

		JMP @xor_exit
