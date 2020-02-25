dgnasm: dgnasm.h *.c
	gcc -fno-builtin-exit -g -rdynamic -o dgnasm dgnasm.c

clean:
	rm -f *.o dgnasm
