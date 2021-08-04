files = ../mcc/tokens.h mcc0.h mcc0.c

mcc0: $(files)
	gcc -fno-builtin-exit -g -rdynamic -DLINUX_COMPAT=1 -o mcc0 mcc0.c
	size mcc0

preproc: $(files)
	gcc -E -P mcc0.c -o preproc_mcc0.c

clean:
	rm -f *.o mcc0 preproc_mcc0.c
