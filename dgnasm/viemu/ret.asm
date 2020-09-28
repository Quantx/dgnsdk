		.text
ret_mask:	077777
ret_exit:	viemu_exit

trap_ret:	LDA 0, t_frame, 3		; Load Frame Pointer into Stack Pointer
		STA 0, t_stack, 3

		LDA 0, @t_stack, 3		; Pop Carry | Program Counter

		MOVL 0, 1
		SUBCL 1, 1
		STA 1, t_cbit, 3		; Update Carry

		LDA 1, ret_mask			; Get just Program Counter
		AND 1, 0

		STA 0, 046			; Store Program Counter in return address
		DSZ 046
		MOV 0, 0

		DSZ t_stack, 3			; Decrement stack pointer
		MOV 0, 0

		LDA 0, @t_stack, 3		; Pop Frame Pointer
		STA 0, t_frame, 3
		STA 0, t_ac3, 3

		DSZ t_stack, 3			; Decrement stack pointer
		MOV 0, 0

		LDA 0, @t_stack, 3		; Pop AC2
		STA 0, t_ac2, 3

		DSZ t_stack, 3			; Decrement stack pointer
		MOV 0, 0

		LDA 0, @t_stack, 3		; Pop AC1
		STA 0, t_ac1, 3

		DSZ t_stack, 3			; Decrement stack pointer
		MOV 0, 0

		LDA 0, @t_stack, 3		; Pop AC0
		STA 0, t_ac0, 3

		DSZ t_stack, 3			; Decrement stack pointer
		MOV 0, 0

		JMP @ret_exit
