
dgnasm: dgnasm.h dgnasm.c mnemonics.c directives.c instructor.c labeler.c utility.c formh.c
	gcc -g -o dgnasm utility.c dgnasm.c mnemonics.c directives.c instructor.c labeler.c formh.c

clean:
	rm -f *.o dgnasm
