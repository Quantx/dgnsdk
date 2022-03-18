START:	INTDS		; Disable interrupts
	READS 3		; AC3 = How many values to compute
	NEGZ 3, 3	; We can only count up
        SUBO 0, 0	; Set AC0 = 0
	INCZ 0, 1	; AC1 = Term 2
LOOP:	MOVZ 0, 2	; AC2 = Next Term
	ADDZ 1, 2
	MOVZ 1, 0	; Term 1 = Term 2
	MOVZ 2, 1
	INCZ 3, 3, SZR
	JMP LOOP
	HALT
	.ENT START
