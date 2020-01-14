#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define DGNASM_VERSION "0.0.1"

// Max length of a source file line
#define MAX_LINE_LENGTH 256

// How much should we print by default
#define DEFAULT_VERBOSITY 4

// The max memory available in the DGN
#define MAX_MEM_SIZE 32768

// Logging constants
#define DGNASM_LOG_INFO 0
#define DGNASM_LOG_WARN 1
#define DGNASM_LOG_SYTX 2
#define DGNASM_LOG_ASEM 3
#define DGNASM_LOG_DBUG 4

// Maximum characters in a label
#define MAX_LABEL_SIZE 16
// Maximum number of back references per label
#define MAX_BACK_REFERENCES 256

// Number of distinct instructions
#define DGNOVA_LANG_LEN 22

// Instruction type constants
#define DGNOVA_INSTR_IOC 0
#define DGNOVA_INSTR_JMP 1
#define DGNOVA_INSTR_MEM 2
#define DGNOVA_INSTR_ARL 3
#define DGNOVA_INSTR_CPU 5

typedef struct dgnasm dgnasm;

typedef struct label
{
    char name[MAX_LABEL_SIZE + 1];
    unsigned short addr;
    struct label * next;
} label;

typedef struct refer
{
    char name[MAX_LABEL_SIZE + 1];
    unsigned short addrs[MAX_BACK_REFERENCES];
    int curAddr;
    struct refer * next;
} refer;

typedef struct instr
{
    char * name;
    unsigned short opcode;
    int type;
} instr;

struct instr dgnlang[DGNOVA_LANG_LEN];

typedef struct dgnasm
{
    // Current file and line
    char * curFile;
    int curLine;

    // Store all symbols
    label * sym;

    // How many opcodes need symbols?
    refer * ref;

    // Buildspace
    unsigned short memory[MAX_MEM_SIZE];

    // Starting address of the program
    unsigned short startAddr;

    // Current address we're working on
    unsigned short curAddr;

    // How much info do we tell the user
    int verbosity;
    // Should we be case sensitive?
    int caseSense;
} dgnasm;

// Primary assembly routine
int assembleFile( char * scrPath, dgnasm * state );

// Utility functions
char * skipWhite( char * str );
int dgnasmcmp( const char * str1, const char * str2, dgnasm * state );
void xlog( int level, dgnasm * state, const char * format, ... );

// Label processing functions
int insertLabel( char * symName, dgnasm * state );

// Instruction processing functions
void initInstructionTable();
struct instr * getInstruction( char * mnem, dgnasm * state );
