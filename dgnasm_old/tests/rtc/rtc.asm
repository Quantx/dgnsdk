		.zero
irqret:		0
irqsub:		inthandle
timer:		9

		.text
		; Interupt routine
inthandle:	NIOS RTC
		DSZ timer
		JMP inthandle_end
		STA 0, timer
		INC 1, 1
inthandle_end:	INTEN
		JMP @irqret

		; Main routine
main:		.ent main
		SUBZL 0, 0	; Load 1 into AC0
		DOAS 0, RTC	; Select 10Hz and start the clock
		MOVL 0, 0
		MOVL 0, 0	; Load 4 into AC0
		COM 0, 1
		MSKO 1		; Enable RTC interupts only
	        MOVOL 0, 0	; Load 9 into AC0
		SUBO 1, 1	; Clear AC1
		INTEN		; Enable interupts
spinloop:	JMP spinloop
