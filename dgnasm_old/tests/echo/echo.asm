	NIOS TTI	; Prime RX
loop:	SKPDN TTI
	JMP . - 1
	DIAS 0, TTI	; RX has a byte
        SKPBZ TTO
	JMP . - 1
	DOAS 0, TTO	; TX is free, transmit new byte
	JMP loop
