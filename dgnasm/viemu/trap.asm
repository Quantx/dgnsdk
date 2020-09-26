		.text
		pc_ptr:		ac0		; Need a temp register to store PC in
		ac3_ptr:   	ac3
		dstsrc_ptr:	dstsrc

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
		STA 3, @ac3_ptr			; Save all accumulators
		LDA 3, ac3_ptr			; Address the rest of the AC save table by ac3_ptr
		STA 2, -1, 3
		STA 1, -2, 3
		STA 0, -3, 3

		SUBCL 0, 0			; Store carry
		STA 0, 1, 3

		; Compute emutbl offset
		LDA 0, @046			; Load trap instruction
		LDA 3, emutbl_mask		; Load trap mask: 11111110
		MOVR 0, 0			; Shift right 3 times
		MOVR 0, 0
		MOVR 0, 0
		ANDZR 0, 3			; AND with mask and shift right final time
		LDA 1, emutbl_ptr
		ADD 1, 3			; Add offset to start of table

		; Get Destination Accumulator (bits 3 & 4 of trap)
		LDA 2, ac_mask
		MOVS 0, 0			; Bits 14-15 now contain DST AC
		AND 0, 2
		MOVS 2, 2			; Prepare AC2 for SRC AC

		; Get Source Accumulator (bits 1 & 2 of trap)
		LDA 1, ac_mask
		MOVR 0, 0
		MOVR 0, 0			; Bits 14-15 now contain SRC AC
		AND 0, 1			; AC1 now contians SRC AC
		ADD 1, 2			; AC2 now contains DST AC and SRC AC
		STA 2, @dstsrc_ptr

		; Enter trap table
		JMP 0, 3
emutbl_mask:	0xFE
ac_mask:	3
emutbl_ptr:	emutbl

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
		trap_norm			; 16
		trap_slld			; 17
		trap_sald			; 18
		trap_slrd			; 19
		trap_sard			; 20
		trap_popj			; 21
		trap_pshr			; 22
		trap_topr			; 23
		trap_topw			; 24
		trap_dsp			; 25
		; Virtual instructions
		trap_sign			; 26
		trap_xor			; 27

		.bss
ac0:		1				; These registers temp store Nova registers
ac1:		1
ac2:		1
ac3:		1
cbit:		1
stack:		1
frame:		1
dstsrc:		1				; Bits 0-7: DST AC, Bits 8-15: SRC AC

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
#include "norm.asm"
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
trap_norm:	1
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
