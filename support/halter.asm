; This is what main() returns to by default
DGNMCC_MAINHALT:	INTDS				; Disable interrupts

			LDA 2, 2, 1			; Load char pointer
			JMP 2, 1
			DGNMCC_MAINHALT_MSG		; Char pointer

DGNMCC_MAINHALT_LOOP:	SKPBZ 0, 11			; Spinlock until character is printed
			JMP -1, 1

			LDA 0, 0, 2			; Load current char

			MOV 0, 0, SNR			; Skip is char is not null
			HALT

			DOAS 0, 11			; Output character

			INC 2, 2			; Increment char pointer
			JMP DGNMCC_MAINHALT_LOOP	; Loop

DGNMCC_MAINHALT_MSG:	15
			12
			'*'
			'*'
			'*'
			40
			'H'
			'A'
			'L'
			'T'
			40
			'*'
			'*'
			'*'
			0
