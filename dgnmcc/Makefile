mcc0_fiiles = mcc0/mcc0/tokenizer.c

files = tokens.h dgnmcc.h dgnmcc.c tokenizer.c declaration.c statement.c expression.c symbols.c types.c debug.c

mcc0:

dgnmcc: $(files)
	gcc -fno-builtin-exit -g -rdynamic -DLINUX_COMPAT=1 -o dgnmcc dgnmcc.c
	size dgnmcc   # Verify size of binary is small enough

debug: $(files)
	gcc -fno-builtin-exit -g -rdynamic -DLINUX_COMPAT=1 -DDEBUG=1 -o dgnmcc dgnmcc.c

preproc: $(files)
	gcc -E -P dgnmcc.c -o preproc_dgnmcc.c

clean:
	rm -f *.o *.seg *.dot dgnmcc a.out preproc_dgnmcc.c
