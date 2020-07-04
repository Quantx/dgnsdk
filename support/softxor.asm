; Software bitwise xor support via: ( !(P & Q) ) & ( !( !P & !Q ) )
DGNMCC_XOR_SUB:		STA 2, DGNMCC_XOR_LFP	; Store frame pointer and return address
			STA 3, DGNMCC_XOR_RET

			MOV 0, 2		; Copy values
			MOV 1, 3

			AND 1, 0		; !(P & Q)
			COM 0, 0

			COM 2, 2		; !( !P & !Q )
			COM 3, 3
			AND 3, 2
			COM 2, 2

			AND 2, 0		; ( !(P & Q) ) & ( !( !P & !Q ) )

			LDA 2, DGNMCC_XOR_LFP	; Load frame pointer and return
			JMP @DGNMCC_XOR_RET
DGNMCC_XOR_LFP:		0
DGNMCC_XOR_RET:		0
