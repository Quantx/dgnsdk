
dgnasm: dgnasm.h dgnasm.c mnemonics.c instructor.c labeler.c utility.c
	gcc -g -std=gnu99 -o dgnasm utility.c dgnasm.c mnemonics.c instructor.c labeler.c

clean:
	rm -f *.o dgnasm
