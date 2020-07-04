;	.LOC	0
	.ZERO

START:	IORST			; Reset I/O Bus
	READS	0		; Get switches to AC0
	LDA	1, DVMSK	; Load Device Mask into AC1
	AND	0, 1		; Mask off Device Bits
	COM	1, 1		; Form Negative of AC1

;  The code fragment from 5 to 11 forms the nucleus of the
; self-modification portion of the program load program. The
; device ID from the switches is complemented and incremented
; in a counter 'til it overflows (goes to 0). The various
; I/O instructions are incremented to the desired device ID
; in said loop.

IOSLP:	ISZ	IOI1		; INC NIOS Instr at 14
	ISZ	IOI2		; INC SKPDN Instr at 30
	ISZ	IOI3		; INC DIAS Instr at 32
	INC	1, 1, SZR	; If loop done, skip - else 5
	JMP	IOSLP		; Loop back

	;    At this point, all the I/O instruction lower 6 bits
	; contain the correct device ID (from the switches) and we
	; can begin the actual boot process. First we load a JMP 377
	; from location 16 and store it into location 377. This sets
	; up the logic for a Data CHannel device load.

	LDA	2, SJMP		; Load self-jump
	STA	2, 0377		; Save at 377
IOI1:	060077			; ((NIOS 0) -1) [1]
	MOVL	0, 0, SZC	; Test for DCH device in SWS
SJMP:	JMP	0377		; Jump + wait for DCH device

	;    The previous JMP 377 is called only if we're loading from
	; a data channel device (i.e. switch 0 was up). The program
	; will loop endlessly at 377 until that location is
	; overwritten by data from the device (it loads 400 octal
	; words). Location 377, once overwritten, becomes the
	; start point for the newly-loaded program.
LEADR:	JSR	GETCH		; Get a character
	MOVC	0, 0, SNR	; Test for NULL
	JMP	LEADR		; Ignore initial NULLs
NXTWRD:	JSR	GPACK		; Get, and pack, bytes
	STA	1, @CURLC	; Save new word thru 26
	ISZ	0100		; Bump word count (from device)
	JMP	NXTWRD		; Get next word

	;    Location 26 is an auto-increment location when accessed
	; via an indirect (deferred) access. The initial value of 77
	; will autoincrement to 100 during the first access from
	; the instruction at location 23. It also serves, at time of
	; initialisation as the mask for the device bits in the console
	; switches. At the end of this program, location 26 will
	; contain entry address of the program just loaded.

DVMSK:
CURLC:	077

	;    Beginning at location 27 is the routine to get bytes from
	; the selected input device and pack them into words for later
	; storage. Remember we're dealing with a big-endian macine
	; here. The routine has two entry points, GPACK and GETCH.
	; GPACK gets, and packs bytes two per word; GETCH gets, and
	; returns a single character in the low-order 8 bits.

GPACK:	SUBZ	1, 1		; Clear AC1 and set carry

GETCH:
IOLP1:
IOI2:	063577			; ((SKPDN 0) -1) [1]
	JMP	IOLP1		; Dev Not Done - Loop
IOI3:	060477			; ((DIAS 0, 0) -1) [1]
	ADDCS	0, 1, SNC	; Pack data from dev into AC1
	JMP	IOLP1		; Word not done - get next char
	MOVS	1, 1		; Word done - swap bytes back
	JMP	0, 3		; Return from JSR

FILLR:	0
