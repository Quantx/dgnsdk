#define t_ac0 0
#define t_ac1 1
#define t_ac2 2
#define t_ac3 3
#define t_cbit 4
#define t_stack 5
#define t_frame 6
#define t_sac 7
#define t_dac 8

		.text
		actbl_ptr:	ac0		; Need a temp register to store PC in
		ac3_ptr:   	ac3

viemu:		.glob viemu			; Entry point, stored in 047
#if TARGET_NOVA < 3
		; Exchange 046 with AC3
		STA 3, @actbl_ptr	 	; Save return address

		LDA 3, 046			; Move AC3 into trap_ac3
		STA 3, @ac3_ptr

		LDA 3, @actbl_ptr		; Move return address into 046
		STA 3, 046
		; System state now mirrors an actual trap instruction
#else
		STA 3, @ac3_ptr			; Save all accumulators
#endif
		LDA 3, actbl_ptr		; AC3 will contain pointer to actbl for rest of routine
		STA 0, t_ac0, 3
		STA 1, t_ac1, 3
		STA 2, t_ac2, 3

		SUBCL 0, 0			; Store carry
		STA 0, t_cbit, 3

		; Compute emutbl offset
		LDA 0, @046			; Load trap instruction
		LDA 2, emutbl_mask		; Load trap mask: 11111110
		MOVR 0, 0			; Shift right 3 times
		MOVR 0, 0
		MOVR 0, 0
		ANDZR 0, 2			; AND with mask and shift right to align trap number in AC2
		LDA 1, emutbl_ptr
		ADD 1, 2			; AC2 now contains pointer into trap table

		; Get Destination Accumulator (bits 3 & 4 of trap)
		LDA 1, ac_mask
		MOVS 0, 0			; Bits 14-15 now contain DST AC number
		AND 0, 1			; AC1 now contains DST AC number
		ADD 3, 1			; AC1 now contains pointer to DST AC
		STA 1, t_dac, 3			; Store pointer to DST AC in dac

		; Get Source Accumulator (bits 1 & 2 of trap)
		LDA 1, ac_mask
		MOVR 0, 0
		MOVR 0, 0			; Bits 14-15 now contain SRC AC number
		AND 0, 1			; AC1 now contians SRC AC  number
		ADD 3, 1			; AC1 now contains pointer to SRC AC
		STA 1, t_sac, 3			; Store pointer to SRC AC in sac

		; Enter trap table
		JMP @0, 2
emutbl_mask:	0xFE
ac_mask:	3
emutbl_ptr:	emutbl

viemu_exit:	; Reset CPU state
		LDA 3, actbl_ptr	; Load pointer to ac table

		LDA 0, t_cbit, 3	; Restore carry
		MOVR 0, 0

		LDA 0, t_ac0, 3		; Restore registers
		LDA 1, t_ac1, 3
		LDA 2, t_ac2, 3
		LDA 3, t_ac3, 3

		ISZ 046			; Return to address after TRAP
		JMP @046

		.data
emutbl:		trap_syscall			; 0
		; Multiply & Divide instructions
		trap_mul			; 1
		trap_div			; 2
		; Nova 3 instructions
		trap_psha			; 3
		trap_popa			; 4
		trap_sav			; 5
		trap_ret			; 6
		trap_mtsp			; 7
		trap_mtfp			; 8
		trap_mfsp			; 9
		trap_mffp			; 10
		; Nova 4 instructions
		trap_ldb			; 11
		trap_stb			; 12
		trap_muls			; 13
		trap_divs			; 14
		; F9445 instructions
		trap_or				; 15
		trap_slld			; 16
		trap_sald			; 17
		trap_slrd			; 18
		trap_sard			; 19
		trap_popj			; 20
		trap_pshr			; 21
		trap_topr			; 22
		trap_topw			; 23
		trap_dsp			; 24
		; Virtual instructions
		trap_sign			; 25
		trap_xor			; 26
		.LOC 101			; Allocate space for rest of table (aprox 128 words)

		.bss
ac0:		1				; These registers temp store Nova registers
ac1:		1
ac2:		1
ac3:		1
cbit:		1
stack:		1
frame:		1
sac:		1				; Points to the source accumulator
dac:		1				; Points to the destination accumulator

#include "syscall.asm"

#if TARGET_NOVA < 2
#include "mul.asm"
#include "div.asm"
#else
		.bss
trap_mul:	1
trap_div:	1
#endif

#if TARGET_NOVA < 3
#include "psha.asm"
#include "popa.asm"
#include "sav.asm"
#include "ret.asm"
#include "mtsp.asm"
#include "mtfp.asm"
#include "mfsp.asm"
#include "mffp.asm"
#else
		.bss
trap_psha:	1
trap_popa:	1
trap_sav:	1
trap_ret:	1
trap_mtsp:	1
trap_mtfp:	1
trap_mfsp:	1
trap_mffp:	1
#endif

#if TARGET_NOVA < 4
#include "ldb.asm"
#include "stb.asm"
#include "muls.asm"
#include "divs.asm"
#else
		.bss
trap_ldb:	1
trap_stb:	1
trap_muls:	1
trap_divs:	1
#endif

#if TARGET_NOVA < 5
#include "or.asm"
#include "slld.asm"
#include "sald.asm"
#include "slrd.asm"
#include "sard.asm"
#include "popj.asm"
#include "pshr.asm"
#include "topr.asm"
#include "topw.asm"
#include "dsp.asm"
#else
		.bss
trap_or:	1
trap_slld:	1
trap_sald:	1
trap_slrd:	1
trap_sard:	1
trap_popj:	1
trap_pshr:	1
trap_topr:	1
trap_topw:	1
trap_dsp:	1
#endif

#include "sign.asm"
#include "xor.asm"
