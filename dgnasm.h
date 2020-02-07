#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define DGNASM_VERSION "1.0.5"

// How many characters are allowed in the filename
#define MAX_FILENAME_LENGTH 255

// Max length of a source file line
#define MAX_LINE_LENGTH 256

// How much should we print by default
#define DEFAULT_VERBOSITY 5

// The max memory available in the DGN
#define MAX_MEM_SIZE 32768

// Logging constants
#define DGNASM_LOG_INFO 0
#define DGNASM_LOG_WARN 1
#define DGNASM_LOG_SYTX 2
#define DGNASM_LOG_ASEM 3
#define DGNASM_LOG_DBUG 4

// Maximum characters in a label
#define MAX_LABEL_SIZE 32

// Maximum number of arguments per instruction
#define MAX_ARGS 16

// Number of distinct instructions
#define DGNOVA_LANG_LEN 29

// Instruction type constants
#define DGNOVA_INSTR_IOC 0 // I/O Instruction
#define DGNOVA_INSTR_JMP 1 // Jump Instruction
#define DGNOVA_INSTR_MEM 2 // Memory Instruction
#define DGNOVA_INSTR_ARL 3 // Arithmetic / Logic Instruction
#define DGNOVA_INSTR_CPC 5 // CPU Control Instruction
#define DGNOVA_INSTR_CPD 6 // CPU Data Instruction

typedef struct label
{
    char name[MAX_LABEL_SIZE + 1];
    unsigned short addr;
    int refCount;
    int isConst;
    struct label * next;
} label;

typedef struct instr
{
    char * name;
    int len;
    unsigned short opcode;
    int type;
} instr;

struct instr dgnlang[DGNOVA_LANG_LEN];
const char * skipCodes[7];

typedef struct dgnasm
{
    // Current file and line
    char * curFile;
    int curLine;

    FILE * listFile;
    FILE * outFile;

    // What pass are we on?
    int curPass;

    // Store all symbols
    label * sym;

    // Buildspace
    unsigned short memory[MAX_MEM_SIZE];

    // Starting address of the program
    unsigned short startAddr;
    // Current address we're working on
    unsigned short curAddr;
    // The entry point for the program
    unsigned short entAddr;

    // How much info do we tell the user
    int verbosity;
    // Should we be case sensitive?
    int caseSense;
    // Format output for simh
    int simhFormat;
    // Should simh auto start the program?
    int simhStart;
} dgnasm;

// Primary assembly routine
int labelFile( char * srcPath, dgnasm * state );
int assembleFile( char * scrPath, dgnasm * state );
int decodeOption( char * arg, dgnasm * state );

// Simh format routine
int formatSimh( dgnasm * state );

// Utility functions
char * skipWhite( char * str );
void stripEnd( char * str );
int computeArgs( char * str, char ** argv );
int dgnasmcmp( const char * str1, const char * str2, dgnasm * state );
int dgnasmncmp( const char * str1, const char * str2, int len, dgnasm * state );
int convertNumber( char * str, unsigned short * val, int halfVal, unsigned short maxVal, dgnasm * state );
void xlog( int level, dgnasm * state, const char * format, ... );
int checkArgs( int argc, int minArg, int maxArg, dgnasm * state );

// Label processing functions
int insertLabel( char * symName, unsigned short symAddr, int isConst, dgnasm * state );
int insertReference( char * symName, unsigned short * outRef, dgnasm * state );
int insertDisplacement( char * symName, unsigned short * outDisp, unsigned short * page, dgnasm * state );
int isLabel(char * symName);

// Opcode table and lookup functions
void initOpcodeTable();
instr * getOpcode( char * mnem, dgnasm * state );

// Assembler directive functions
int processDirective( char * direct, int argc, char ** args, int labelHere, dgnasm * state );

// Instruction generation functions
int buildInstruction( instr * curIns, char * fpos, int argc, char ** args, dgnasm * state );
