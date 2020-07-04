		.LOC 100

		INTDS		; Disable interrupt
		LDA 0, asciibeg	; Load constants
		LDA 1, asciiend

mainloop:	SKPBZ 0, 11	; Wait untill TTO is free
		JMP -1, 1

		DOAS 0, 11	; Output to TTO

		INCZ 0, 0	; Increment char to be printed

		SUBO# 0, 1, SNR	; Check if end of ascii table
		LDA 0, asciibeg	; Reset starting constant
		JMP mainloop	; Loop

asciibeg:	40
asciiend:	177
