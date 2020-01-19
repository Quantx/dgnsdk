#include "dgnasm.h"

int insertLabel( char * symName, unsigned short symAddr, int isConst, struct dgnasm * state )
{
    int i;

    char * modeName = "label";
    if ( isConst ) modeName = "constant";

    label * curSym;

    // Make sure the label starts with a letter
    if ( !isLabel( symName ) )
    {
        xlog( DGNASM_LOG_SYTX, state, "%s definition '%s' does not start with a letter\n", modeName, symName );
    }

    // Make sure the label is a valid length
    if ( strlen( symName ) > MAX_LABEL_SIZE )
    {
        xlog( DGNASM_LOG_SYTX, state, "%s definition '%s' excedes char limit of %d\n", modeName, symName, MAX_LABEL_SIZE );
        return 0;
    }

    // Check for whitespace in the label
    for ( i = 0; symName[i] != '\0'; i++ )
    {
        if ( symName[i] == ' ' || symName[i] == '\t' )
        {
            xlog( DGNASM_LOG_WARN, state, "whitespace present in %s definition '%s'\n", modeName, symName );
            break;
        }
    }

    // Check if this is a duplicate definition
    for ( curSym = state->sym; curSym != NULL; curSym = curSym->next )
    {
        if ( !dgnasmcmp( symName, curSym->name, state ) )
        {
            xlog( DGNASM_LOG_SYTX, state, "duplicate symbol definition '%s'\n", symName );
            return 0;
        }
    }

    // Create symbol
    curSym = malloc(sizeof(label));
    strcpy( curSym->name, symName );
    curSym->addr = symAddr;
    curSym->refCount = 0;
    curSym->isConst = isConst;
    curSym->next = NULL;

    // Add symbol to table
    label ** firSym;
    for ( firSym = &(state->sym); *firSym != NULL; firSym = &((*firSym)->next) );
    *firSym = curSym;


    xlog( DGNASM_LOG_DBUG, state, "Added %s '%s' at [%05o] to symbol table\n", modeName, curSym->name, symAddr );

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

int insertDisplacement( char * symName, unsigned short * outDisp, unsigned short * page, dgnasm * state )
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
    if ( *page > 3 )
    {
        *page = curSym->addr > 0xFF;
    }

    // Page 0 displacement
    if ( *page == 0 )
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
        xlog( DGNASM_LOG_DBUG, state, "added a Page 0 displacement of %d at [%05o] to label '%s'\n",
              outVal, state->curAddr, curSym->name );

        *outDisp |= outVal & 0xFF;
    }
    // PC displacement
    else if ( *page == 1 )
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
        xlog( DGNASM_LOG_DBUG, state, "added a PC displacement of %d at [%05o] to label '%s'\n",
              outVal, state->curAddr, curSym->name );
    }
    else
    {
        xlog( DGNASM_LOG_SYTX, state, "cannot use page mode %d with a label\n", *page );
        return 0;
    }

    curSym->refCount++;

    return 1;
}

int isLabel( char * symName )
{
    return (*symName >= 'A' && *symName <= 'Z') || (*symName >= 'a' && *symName <= 'z');
}
