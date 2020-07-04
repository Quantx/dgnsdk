			.ENT main
			.LOC 2
DGNMCC_BASE_PTR:	DGNMCC_BUILD_BASE_PTR		; Stores start of currently running program
DGNMCC_PUSH:		DGNMCC_PUSH_SUB			; Stores location of push subroutine
DGNMCC_XOR:		DGNMCC_XOR_SUB			; Stores location of xor subroutine
			.LOC 30
DGNMCC_STACK_PTR:
DGNMCC_POP:		DGNMCC_BUILD_STACK		; Stores stack pointer
			.LOC 40
			.LOC 400
			.INCLUDE "support/softstack.asm"; Software stack routine
			.INCLUDE "support/softxor.asm"	; Software xor routine
			.INCLUDE "support/ttyio.asm"	; Include basic I/O routines
			.INCLUDE "support/halter.asm"	; Include stack catcher
DGNMCC_BUILD_BASE_PTR:	DGNMCC_BUILD_STACK
			0
			.INCLUDE "build/compiled.asm" 	; The compiled program
			DGNMCC_MAINHALT
			0
DGNMCC_BUILD_STACK:	0
