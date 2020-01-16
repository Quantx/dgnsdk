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

    // Create symbol
    curSym = malloc(sizeof(label));
    strcpy( curSym->name, symName );
    curSym->addr = state->curAddr;
    curSym->refCount = 0;
    curSym->next = NULL;

    // Add symbol to table
    label ** firSym;
    for ( firSym = &(state->sym); *firSym != NULL; firSym = &((*firSym)->next) );
    *firSym = curSym;


    xlog( DGNASM_LOG_DBUG, state, "Added label '%s' at [%05o] to symbol table\n", curSym->name, curSym->addr );

    return 1;
}

int insertReference( char * symName, unsigned short * outRef, dgnasm * state )
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

    // Output reference
    *outRef |= curSym->addr;
    xlog( DGNASM_LOG_DBUG, state, "added reference at [%05o] to label '%s'\n",
          state->curAddr, curSym->name );

    // Increase reference count
    curSym->refCount++;

    return 1;
}

int insertDisplacement( char * symName, unsigned short * outDisp, int page, dgnasm * state )
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

    // Auto-page
    if ( page > 3 && curSym->addr < 0xFF ) page = 0;
    else page = 1;

    // Page 0 displacement
    if ( page == 0 )
    {
        unsigned short outVal = curSym->addr;

        // Forward refs only
        if ( outVal > 0xFF )
        {
            xlog( DGNASM_LOG_ASEM, state, "forward reference to label '%s' has a displacement of %d which excedes maximum of 127d\n",
                  symName, state->curAddr - curSym->addr );
            return 0;
        }

        // Output reference
        xlog( DGNASM_LOG_DBUG, state, "added a Page 0 displacement %d at [%05o] to label '%s'\n",
              outVal, state->curAddr, curSym->name );

        *outDisp |= outVal & 0xFF;
    }
    // PC displacement
    else if ( page == 2 )
    {
        short outVal = curSym->addr - state->curAddr;

        // Forward ref
        if ( outVal > 127 )
        {
            xlog( DGNASM_LOG_ASEM, state, "forward reference to label '%s' has a displacement of %d which excedes maximum of 127d\n",
                  symName, state->curAddr - curSym->addr );
            return 0;
        }
        // Back ref
        else if ( outVal < -128 )
        {
            xlog( DGNASM_LOG_ASEM, state, "forward reference to label '%s' has a displacement of %d which excedes minimum of -128d\n",
                  symName, state->curAddr - curSym->addr );
            return 0;
        }

        // Output reference
        *outDisp |= ((unsigned short)outVal) & 0xFF;
        xlog( DGNASM_LOG_DBUG, state, "added a PC displacement %d at [%05o] to label '%s'\n",
              outVal, state->curAddr, curSym->name );
    }

    curSym->refCount++;

    return 1;
}

int isLabel( char * symName )
{
    return (*symName > 'A' && *symName < 'Z') || (*symName > 'a' && *symName < 'z');
}
