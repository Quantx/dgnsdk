; Spinlock until the tty is ready for a new character then write to it
; extern void tty_putc( char output );
tty_putc:	SKPBZ 0, 11		; Check if tty output done flag is 1 then skip
		JMP -1, 1		; Spinlock

                LDA 0, @DGNMCC_POP	; Get char to output
                DOAS 0, 11		; Write char to tty output and set done flag

		SUBO 0, 0		; Set default return value
                LDA 3, 0, 2		; Load current lfp
		STA 3, DGNMCC_STACK_PTR	; Adjust stack
		LDA 3, @DGNMCC_POP	; Load old lfp from stack
		STA 3, 0, 2		; Update current lfp with old lfp
		LDA 3, @DGNMCC_POP	; Load return address
		JMP 0, 3		; Exit subroutine

; Spinlock until the tty has a new character for us then read it
; extern char tty_getc( );
tty_getc:	SKPDN 0, 10		; If there is not a char in the buffer ensure busy flag is set
		NIOS 0, 10		; Set busy flag
		SKPDN 0, 10		; Check if tty input done flag is 1 then skip
		JMP -1, 1		; Spinlock

		DIAS 0, 10		; Read character from tty keyboard

		LDA 3, 0, 2		; Load current lfp
                STA 3, DGNMCC_STACK_PTR	; Adjust stack
                LDA 3, @DGNMCC_POP	; Load old lfp from stack
                STA 3, 0, 2		; Update current lfp with old lfp
		LDA 3, @DGNMCC_POP	; Load return address
                JMP 0, 3		; Exit subroutine
