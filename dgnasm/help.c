char * helpdump = "about:\r\n"
"    ` - Data General Nova Assembler (c) 2019 - 2020 Samuel Deutsch\r\n"
"    full source code available here: https://github.com/Quantx/dgnasm\r\n"
"\r\n"
"usage:\r\n"
"    ` [options] source1.asm source2.asm ... sourceX.asm\r\n"
"\r\n"
"options:\r\n"
"    -mh	Output for SimH's Data General Nova Emulator's load format\r\n"
"    -ma	Output for SimH's Data General Nova Emulator's load format with auto-start\r\n"
"    -mv	Output for Data General Nova 4's Virtual Console cell format\r\n"
"    -t [1-31]	Specify stack size for the program: Stack Size (in words) = N * 1024 - 256\r\n"
"    -g		Force all labels to be global\r\n";

void showhelp( char * progname )
{
    char * hsp = helpdump;
    int i = -1, pnl = 0;

    while ( progname[pnl] ) pnl++;

    while ( i < 0 || hsp[i] )
    {
        hsp += i + 1; i = 0;

        while ( hsp[i] && hsp[i] != '`' ) i++;

        write( 1, hsp, i );

        if ( hsp[i] == '`' ) write( 1, progname, pnl );
    }

    exit(1);
}
