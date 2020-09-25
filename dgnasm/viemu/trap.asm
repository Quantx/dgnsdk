		.text
		pc_ptr:
		ac0_ptr:	ac0		; Need a temp register to store PC in
		ac1_ptr:	ac1
		ac2_ptr:	ac2
		ac3_ptr:   	ac3
		cbit_ptr:	cbit
		stack_ptr:	stack
		frame_ptr:	frame

viemu:		.glob viemu			; Entry point, stored in 047
#if TARGET_NOVA < 3
		; Exchange 046 with AC3
		STA 3, @pc_ptr			; Save return address

		LDA 3, 046			; Move AC3 into trap_ac3
		STA 3, @ac3_ptr

		LDA 3, @pc_ptr			; Move return address into 046
		STA 3, 046

		LDA 3, @ac3_ptr			; Recover AC3
		; System state now mirrors an actual trap instruction
#endif
		STA 0, ac0_ptr			; Save all accumulators
		STA 1, ac1_ptr
		STA 2, ac2_ptr
		STA 3, ac3_ptr

		ADCL 0, 0			; Store Carry
		STA 0, @cbit_ptr

		; Jump into trap table
		LDA 0, @046			; Load trap instruction
		LDA 3, emutbl_mask		; Load trap mask: 11111110
		MOVR 0, 0			; Shift right 3 times
		MOVR 0, 0
		MOVR 0, 0
		ANDR 0, 3			; AND with mask and shift right final time
		LDA 0, @emutbl_ptr
		ADD 0, 3			; Add offset to start of table
		JMP 0, 3			; Enter table
emutbl_mask:	0xFE
emutbl_ptr:	emutbl

               .bss
ac0:            1                               ; These registers temp store Nova registers
ac1:            1
ac2:            1
ac3:            1
cbit:           1
stack:          1
frame:          1

		.data
emutbl:		;syscall
		; Multiply & Divide instructions
		mul
		div
		; Nova 3 instructions
		; Nova 4 instructions
		ldb
		stb
		muls
		divs
		; F9445 instructions
		; Virtual instructions

#if TARGET_NOVA < 2
#include <mul.asm>
#include <div.asm>
#else
		.bss
mul:		1
div:		1
#endif


#if TRAGET_NOVA < 4
#include <ldb.asm>
#include <stb.asm>
#include <muls.asm>
#include <divs.asm>
#else
		.bss
ldb:		1
stb:		1
muls:		1
divs:		1
#endif
