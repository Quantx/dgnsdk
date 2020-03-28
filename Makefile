dgnasm: dgnasm.h dgnasm.c segments.c tokenizer.c assembler.c help.c
	gcc -fno-builtin-exit -g -rdynamic -o dgnasm dgnasm.c

symbols: dgnasm.h symbols.c
	gcc -fno-builtin-exit -g -rdynamic -o build_symbols symbols.c

clean:
	rm -f *.o dgnasm
