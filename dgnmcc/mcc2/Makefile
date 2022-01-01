files = ../mcc/tokens.h ../mcc/statements.h ../mcc/opcodes.h mcc2.h mcc2.c node.c function.c statement.c expression.c regalloc.c generator.c

mcc2: $(files)
	gcc -fno-builtin-exit -g -rdynamic -DLINUX_COMPAT=1 -o mcc2 mcc2.c
	size mcc2   # Verify size of binary is small enough

debug: $(files)
	gcc -fno-builtin-exit -g -rdynamic -DLINUX_COMPAT=1 -DDEBUG=1 -o mcc2 mcc2.c

preproc: $(files)
	gcc -E -P mcc2.c -o preproc_mcc2.c

clean:
	rm -f *.o *.seg *.dot mcc2 a.out preproc_mcc2.c
