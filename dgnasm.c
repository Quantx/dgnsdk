#include "dgnasm.h"

int main( int argc, char * argv[] )
{
    if ( argc <= 1 )
    {
        printf(
"    DGNASM - Data General Nova Assembler (c) 2019\n"
"    Version %s by Samuel Deutsch, https://github.com/Quantx/dgnasm\n\n"
"Syntax:\n"
"    dgnasm [options] input_file\n"
"    dgnasm [options] input_file output_file\n"
"    dgnasm [options] input_file list_file out_file\n\n"
"Options:\n"
"    -c    Enable case sensitivity\n"
        , DGNASM_VERSION );
        return 1;
    }

    // Generate the Nova ASM table
    initInstructionTable();

    dgnasm state;

    state.sym = NULL;
    state.ref = NULL;

    memset(state.memory, 0, MAX_MEM_SIZE);

    state.startAddr = 0;
    state.curAddr = 0;

    state.verbosity = DEFAULT_VERBOSITY;
    state.caseSense = 0;

    assembleFile( argv[argc - 1], &state );
}

int assembleFile( char * srcPath, dgnasm * state )
{
    // Store debug info regarding the current file
    state->curFile = srcPath;
    state->curLine = 0;

    // Open the source file
    FILE * srcFile = fopen( srcPath, "r" );

    // Make sure the file exists
    if ( srcFile == NULL )
    {
        printf( "Unable to open source file '%s'", srcPath );
        return 0;
    }

    int i;

    label * curSym;
    refer * curRef;
    refer * lastRef;
    instr * curIns;

    // Store current line of file
    char line[MAX_LINE_LENGTH + 1];
    char * ipos = NULL; // Instruction position
    char * apos = NULL; // Argument position

    while ( fgets( line, MAX_LINE_LENGTH + 1, srcFile ) != NULL )
    {
        // Increment current line number
        state->curLine++;

        // Terminate on comments or newlines
        for ( i = 0; line[i] != '\0' && line[i] != ';' && line[i] != '\n'; i++ );
        line[i] = '\0';

        // Check if a label exists on this line
        ipos = strchr( line, ':' );

        if ( ipos != NULL )
        {
            // Replace label terminator with string terminator
            *ipos = '\0';
            ipos++;

            // Check for extra label terminators
            if ( strchr(ipos, ':') != NULL )
            {
                xlog( DGNASM_LOG_SYTX, state, "one colon per line please\n" );
                fclose( srcFile );
                return 0;
            }

            // Insert the label into the symbol table
            if ( !insertLabel( line, state ) )
            {
                fclose( srcFile );
                return 0;
            }
        }

        // Get start of the opcode
        ipos = skipWhite( ipos );

        // Terminate end of opcode
        for ( i = 0; ipos[i] != '\0' && ipos[i] != ' ' && ipos[i] != '\t'; i++ );
        if ( ipos[i] != '\0' )
        {
            ipos[i] = '\0';
            // Setup argument pointer
            apos = skipWhite(ipos + i + 1);
        }

        // Look up instruction
        curIns = getInstruction( ipos, state );

        // Check for LOC assembler directives
        if ( !dgnasmcmp( ipos, ".LOC", state )
          || !dgnasmcmp( ipos, ".ORG", state ) )
        {

        }
        // Check for opcode
        else if ( curIns != NULL )
        {
            
        }
        // Check for constant
        else
        {

        }
    }

    // Don't forget to close the file
    fclose( srcFile );
    return 1;
}
