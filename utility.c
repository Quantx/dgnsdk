int datcmp( unsigned char * addr1, unsigned char * addr2, int limit )
{
    int i = 0;
    while ( i < limit )
    {
        if ( addr1[i] < addr2[i] ) return -1;
        if ( addr1[i] > addr2[i] ) return  1;

        i++;
    }

    return 0;
}

void * datcpy( unsigned char * dst, unsigned char * org, int limit )
{
    int i = 0;
    while ( i < limit ) dst[i] = org[i++];
    return dst;
}
