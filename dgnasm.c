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
"    dgnasm [options] input_file output_file list_file\n\n"
"Options:\n"
"    -c    Enable case sensitivity\n"
        , DGNASM_VERSION );
        return 0;
    }

    dgnasm state;

    state.sym = NULL;

    memset(state.memory, 0, MAX_MEM_SIZE);

    state.startAddr = 0;
    state.curAddr = 0;

    state.verbosity = DEFAULT_VERBOSITY;
    state.caseSense = 0;

    char inPath[MAX_FILENAME_LENGTH + 1] = "";
    char listPath[MAX_FILENAME_LENGTH + 1] = "";
    char outPath[MAX_FILENAME_LENGTH + 1] = "";


    int i;
    for ( i = 1; i < argc; i++ )
    {
        char * curArg = argv[i];
        if ( curArg[0] == '-' )
        {
            //decodeOption( arg, &state );
        }
        else
        {
            if ( strlen( curArg ) == MAX_FILENAME_LENGTH + 1 )
            {
                printf( "File Error: name %s, excedes the maximum character limit of %d\n", curArg, MAX_FILENAME_LENGTH );
                return 1;
            }
            else if ( inPath[0] == '\0' )
            {
                strcpy( inPath, curArg );
            }
            else if ( outPath[0] == '\0' )
            {
                strcpy( outPath, curArg );
            }
            else if ( listPath[0] == '\0' )
            {
                strcpy( listPath, curArg );
            }
            else
            {
                printf( "Too many file names specified!\n" );
                return 1;
            }
        }
    }

    // Make sure we got an input file
    if ( inPath[0] == '\0' )
    {
        printf( "File Error: no input file specified!" );
        return 1;
    }

    char * fext;
    // Generate output file path if needed
    if ( outPath[0] == '\0' )
    {
        strcpy( outPath, inPath );
        fext = strrchr( outPath, '.' ) + 1;
        strcpy( fext, "bin" );
    }

    // Generate listing file path if needed
    if ( listPath[0] == '\0' )
    {
        strcpy( listPath, inPath );
        fext = strrchr( listPath, '.' ) + 1;
        strcpy( fext, "lst" );
    }

    // Open list file
    state.listFile = fopen( listPath, "w" );

    if ( state.listFile == NULL )
    {
        printf( "File Error: unable to open list file '%s'\n", listPath );
        return 1;
    }

    // Open output file
    state.outFile = fopen( outPath, "wb" );

    if ( state.outFile == NULL )
    {
        printf( "File error: unable to open output file '%s'\n", outPath );
        return 1;
    }

    // Generate the Nova ASM table
    initOpcodeTable();

    // Assemble the input file
    assembleFile( inPath, &state );

    // Final output
    void * progStart = state.memory + state.startAddr;
    int progLen = state.curAddr - state.startAddr;

    fwrite( progStart, sizeof(short), progLen, state.outFile );

    //generateListing( &state );

    // Close files
    fclose( state.listFile );
    fclose( state.outFile );

    return 0;
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
        printf( "File Error: Unable to open source file '%s'\n", srcPath );
        return 0;
    }

    int i;

    int labelHere; // Does this line have a label on it?

    label * curSym;
    instr * curIns;

    // Store arguments
    char * argv[MAX_ARGS];
    int argc;

    // Store current line of file
    char line[MAX_LINE_LENGTH + 1];
    char * ipos = NULL; // Instruction position
    char * apos = NULL; // Argument position

    xlog( DGNASM_LOG_DBUG, state, "Starting labeling pass\n" );

    // Labeling pass
    while ( fgets( line, MAX_LINE_LENGTH + 1, srcFile ) != NULL )
    {
        state->curLine++;

        // Terminate on comments or newlines
        for ( i = 0; line[i] != '\0' && line[i] != ';' && line[i] != '\n'; i++ );
        line[i] = '\0';

        // Check if a label exists on this line
        ipos = strchr( line, ':' );
        labelHere = (ipos != NULL);

        if ( labelHere )
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
        else
        {
            ipos = line;
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

        // Compute arguments
        argc = computeArgs( apos, argv );
        if ( argc >= MAX_ARGS )
        {
            xlog( DGNASM_LOG_SYTX, state, "Exceded the maximum of %d arguments\n", MAX_ARGS );
            fclose( srcFile );
            return 0;
        }

        // Check for LOC assembler directives
        if ( !dgnasmcmp( ipos, ".LOC", state )
          || !dgnasmcmp( ipos, ".ORG", state ) )
        {
            if ( labelHere )
            {
                xlog( DGNASM_LOG_SYTX, state, "No labels before .org or .loc directives\n" );
                fclose( srcFile );
                return 0;
            }

            // Check number of arguments
            if ( !checkArgs( argc, 1, 1, state ) )
            {
                fclose( srcFile );
                return 0;
            }

            // Get new address to start compilation from
            unsigned short newAddr;
            if ( !convertNumber( argv[0], &newAddr, 0, 32767, state ) ) return 0;

            // Check to make sure we're not going backwards
            if ( newAddr < state->curAddr )
            {
                xlog( DGNASM_LOG_SYTX, state, "New %s [%o] is behind current address [%o], must be after!\n", ipos, newAddr, state->curAddr );
                fclose( srcFile );
                return 0;
            }

            // Check if we're changing the start address
            if ( state->curAddr == state->startAddr )
            {
                xlog( DGNASM_LOG_DBUG, state, "Moved start address of program to [%o]\n", newAddr );
                state->startAddr = newAddr;
            }

            // Update the current address
            state->curAddr = newAddr;
        }
    }

    // Reset file pointer to start
    fseek( srcFile, 0, SEEK_SET );
    state->curLine = 0;
    state->curAddr = state->startAddr;

    xlog( DGNASM_LOG_DBUG, state, "Starting assembly pass\n" );

    // Assembly pass
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
            *ipos = '\0';
            ipos++;
        }
        else
        {
            ipos = line;
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

        // Look up opcode
        curIns = getOpcode( ipos, state );

        // Compute arguments
        argc = computeArgs( apos, argv );
        if ( argc >= MAX_ARGS )
        {
            xlog( DGNASM_LOG_SYTX, state, "Exceded the maximum of %d arguments\n", MAX_ARGS );
            fclose( srcFile );
            return 0;
        }

        // Check for LOC assembler directives
        if ( !dgnasmcmp( ipos, ".LOC", state )
          || !dgnasmcmp( ipos, ".ORG", state ) )
        {
            // Get new address to start compilation from
            unsigned short newAddr;
            if ( !convertNumber( argv[0], &newAddr, 0, 32767, state ) ) return 0;

            xlog( DGNASM_LOG_DBUG, state, "Moved up %d address from [%o] to [%o]\n", newAddr - state->curAddr, state->curAddr, newAddr );

            // Update the current address
            state->curAddr = newAddr;
        }
        // Check if this is an instruction
        else if ( curIns != NULL )
        {
            // Move to instruction pointer to flags
            ipos += curIns->len;

            // Compute opcode
            buildInstruction( curIns, ipos, argc, argv, state );

            // Increment to the next address
            state->curAddr++;
        }
        // Check for constant
        else
        {
            unsigned short data = 0;

            // Set indirect flag
            if ( ipos[0] == '@' )
            {
                ipos++;
                data |= 1 << 15;
            }

            // Label constant
            if ( isLabel( ipos ) )
            {
                if ( insertReference( ipos, 0, state ) )
                {
                    fclose( srcFile );
                    return 0;
                }
            }
            // Literal constant
            else
            {
                if ( data > 0 )
                {
                    xlog( DGNASM_LOG_WARN, state, "Indirect flag '@' set on a literal constant\n" );
                }

                unsigned short litVal;
                if ( !convertNumber( ipos, &litVal, 0, 0xFFFF, state ) )
                {
                    fclose( srcFile );
                    return 0;
                }
                data |= litVal;
            }

            // Increment to the next address
            state->curAddr++;
        }
    }

    // Don't forget to close the file
    fclose( srcFile );
    return 1;
}

int generateListing( dgnasm * state )
{

}

int deocdeOption( char * arg, dgnasm * state )
{
    return 1;
}
