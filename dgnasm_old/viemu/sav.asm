		.text
sav_mask:	0077777
sav_exit:	viemu_exit

trap_sav:	ISZ t_stack, 3		; Increment stack pointer
		MOV 0, 0

		LDA 0, t_ac0, 3		; Push AC0
		STA 0, @t_stack, 3

		ISZ t_stack, 3		; Increment stack pointer
		MOV 0, 0

		LDA 0, t_ac1, 3		; Push AC1
		STA 0, @t_stack, 3

		ISZ t_stack, 3		; Increment stack pointer
		MOV 0, 0

		LDA 0, t_ac2, 3		; Push AC2
		STA 0, @t_stack, 3

		ISZ t_stack, 3		; Increment stack pointer
		MOV 0, 0

		LDA 0, t_frame, 3	; Push Frame Pointer
		STA 0, @t_stack, 3

		ISZ t_stack, 3		; Increment stack pointer
		MOV 0, 0

		LDA 0, t_ac3, 3		; Get bits 1-15 of AC3
		LDA 1, sav_mask
		AND 1, 0

		LDA 1, t_cbit, 3	; Add carry to AC3
		MOVZR 1, 1
		MOVZR 1, 1
		ADD 1, 0

		STA 0, @t_stack, 3	; Push CBIT | AC3

		LDA 0, t_stack, 3	; Load Stack Pointer into both Frame Pointer and AC3
		STA 0, t_frame, 3
		STA 0, t_ac3, 3

		JMP @sav_exit
