files = ../mcc/tokens.h ../mcc/statements.h mcc1.h mcc1.c tokenizer.c declaration.c statement.c expression.c symbols.c types.c debug.c

mcc1: $(files)
	gcc -fno-builtin-exit -g -rdynamic -DLINUX_COMPAT=1 -o mcc1 mcc1.c
	size mcc1   # Verify size of binary is small enough

debug: $(files)
	gcc -fno-builtin-exit -g -rdynamic -DLINUX_COMPAT=1 -DDEBUG=1 -o mcc1 mcc1.c

preproc: $(files)
	gcc -E -P mcc1.c -o preproc_mcc1.c

clean:
	rm -f *.o *.seg *.dot mcc1 a.out preproc_mcc1.c
