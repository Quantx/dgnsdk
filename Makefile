dgnasm: dgnasm.h *.c
	gcc -g -rdynamic -o dgnasm dgnasm.c

clean:
	rm -f *.o dgnasm
