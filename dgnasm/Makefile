dgnasm: dgnasm.h dgnasm.c segments.c tokenizer.c assembler.c help.c symbols.c
	gcc -fno-builtin-exit -g -rdynamic -DLINUX_COMPAT=1 -o dgnasm dgnasm.c

clean:
	rm -f *.o dgnasm build_symbols

