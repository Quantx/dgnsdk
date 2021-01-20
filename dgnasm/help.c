int8_t * helpdump = "about:\r\n"
"    ` - Data General Nova Assembler (c) 2019 - 2020 Samuel Deutsch\r\n"
"    full source code available here: https://github.com/Quantx/dgnsdk\r\n"
"\r\n"
"usage:\r\n"
"    ` [options] source1.asm source2.asm ... sourceX.asm\r\n"
"\r\n"
"options:\r\n"
"    -mh		Output for SimH's Data General Nova Emulator's load format\r\n"
"    -ma		Output for SimH's Data General Nova Emulator's load format with auto-start\r\n"
"    -mv		Output for Data General Nova 4's Virtual Console cell format\r\n"
"    -t [1-31]		Specify stack size for the program: Stack Size (in words) = N * 1024 - 256\r\n"
"    -g			Force all labels to be global\r\n"
"    -n [b|m|3|4|5]     Compiler target, specifies which Nova unit to build for\r\n"
"				b (default) = base nova instruction set\r\n"
"				m = base nova instruction set with multiply and divide\r\n"
"				3 = Nova 3 instruction set\r\n"
"				4 = Nova 4 instruction set\r\n"
"				5 = Fairchild F9445 instruction set\r\n"
"    -v			Enable virtual instruction emulation\r\n"
; // Terminate help string

void showhelp( int8_t * progname )
{
    int8_t * hsp = helpdump;
    int16_t i = -1, pnl = 0;

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
