dgnasm: dgnasm.h *.c
	gcc -g -rdynamic -o dgnasm *.c

clean:
	rm -f *.o dgnasm
