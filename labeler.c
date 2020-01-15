#include "dgnasm.h"

int insertLabel( char * symName, struct dgnasm * state )
{
    int i;

    label * curSym;

    // Make sure the label starts with a letter
    if ( !isLabel( symName ) )
    {
        xlog( DGNASM_LOG_SYTX, state, "label definition '%s' does not start with a letter\n", symName );
    }

    // Make sure the label is a valid length
    if ( strlen( symName ) > MAX_LABEL_SIZE )
    {
        xlog( DGNASM_LOG_SYTX, state, "label definition '%s' excedes char limit of %d\n", symName, MAX_LABEL_SIZE );
        return 0;
    }

    // Check for whitespace in the label
    for ( i = 0; symName[i] != '\0'; i++ )
    {
        if ( symName[i] == ' ' || symName[i] == '\t' )
        {
            xlog( DGNASM_LOG_WARN, state, "whitespace present in label definition '%s'\n", symName );
            break;
        }
    }

    // Check if this is a duplicate definition
    for ( curSym = state->sym; curSym != NULL; curSym = curSym->next )
    {
        if ( !dgnasmcmp( symName, curSym->name, state ) )
        {
            xlog( DGNASM_LOG_SYTX, state, "duplicate label definition '%s'\n", symName );
            return 0;
        }
    }

    // Add symbol to table
    curSym = malloc(sizeof(label));
    strcpy( curSym->name, symName );
    curSym->addr = state->curAddr;
    curSym->next = NULL;

    return 1;
}

int insertReference( char * symName, int halfRef, dgnasm * state )
{
    label * curSym;

    for ( curSym = state->sym; curSym != NULL; curSym = curSym->next )
    {
        if ( !dgnasmcmp( symName, curSym->name, state ) ) break;
    }

    // Make sure requested symbol exists
    if ( curSym == NULL )
    {
        xlog( DGNASM_LOG_ASEM, state, "reference to a non-existant label '%s'\n", symName );
        return 0;
    }

    // Get output location
    unsigned short * outAddr = state->memory + state->curAddr;

    // A single byte displacemen
    if ( halfRef )
    {

    }
    // A full word address
    else
    {
        *outAddr |= curSym->addr;
    }

    return 1;
}

int isLabel( char * symName )
{
    return (symName[0] > 'A' && symName[0] < 'Z') || (symName[0] > 'a' && symName[122] < 'z');
}
