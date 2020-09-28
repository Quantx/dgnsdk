		.text
viemu_ptr:	viemu
bit_pat:	0xAA55
bit_pat_ptr:    >bit_pat
lsb_empty_ptr:	>lsb_empty
msb_empty_ptr:	<msb_empty

start:		.ent start

		LDA 0, viemu_ptr	; Initialize Virtual Instruction Emulation
		STA 0, 047

		LDA 2, bit_pat
		MOVOS 2, 3

		LDA 0, lsb_empty_ptr
		LDA 1, bit_pat_ptr

		LDB 1, 1
		STB 1, 0
		HALT

		.data
lsb_empty:	0xFF00
msb_empty:	0x00FF

#include "../../viemu/trap.asm"
