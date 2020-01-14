#include "dgnasm.h"

int insertLabel( char * symName, struct dgnasm * state )
{
    int i;

    label * curSym;
    refer * curRef;
    refer * lastRef;

    // Make sure the label is a valid length
    if ( strlen( symName ) > MAX_LABEL_SIZE )
    {
        xlog( 2, state, "label definition '%s' excedes char limit of %d\n", symName, MAX_LABEL_SIZE );
        return 0;
    }

    // Check for whitespace in the label
    for ( i = 0; symName[i] != '\0'; i++ )
    {
        if ( symName[i] == ' ' || symName[i] == '\t' )
        {
            xlog( 1, state, "whitespace present in label definition '%s'\n", symName );
            break;
        }
    }

    // Check if this is a duplicate definition
    for ( curSym = state->sym; curSym != NULL; curSym = curSym->next )
    {
        if ( !dgnasmcmp( symName, curSym->name, state ) )
        {
            xlog( 2, state, "duplicate label definition '%s'\n", symName );
            return 1;
        }
    }

    // Add symbol
    curSym = malloc(sizeof(label));
    strcpy( symName, curSym->name );
    curSym->addr = state->curAddr;
    curSym->next = NULL;

    lastRef = NULL;
    // Check if any past opcodes are referencing us
    for ( curRef = state->ref; curRef != NULL; curRef = curRef->next )
    {
        if ( !dgnasmcmp( curSym->name, curRef->name, state ) )
        {
            // Update back references
            for ( i = 0; i < curRef->curAddr; i++ )
            {
                //TODO Make old OPCODES point to us
            }

            // Delete back reference
            if ( lastRef != NULL ) lastRef->next = curRef->next;
            free(curRef);

            break;
        }

        lastRef = curRef;
    }

    return 1;
}
