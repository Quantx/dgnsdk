LDA 2, @DGNMCC_BASE_PTR
LDA 3, 2, 1 ; Load local variable
ADDZ 2, 3, SKP
0d
LDA 0, 0, 3
MOV 0, 3 ; Dereference
LDA 0, 0, 3
MOV 0, 0, SZR ; While statement
JMP 6, 1
LDA 2, DGNMCC_BASE_PTR
LDA 3, 3, 1
ADDZ 2, 3
JMP 0, 3
55d
LDA 2, DGNMCC_BASE_PTR
LDA 0, @DGNMCC_BASE_PTR ; Setup return fp and pc for function
LDA 3, DGNMCC_STACK_PTR
INC 3, 3
STA 3, DGNMCC_STACK_PTR
JSR @DGNMCC_PUSH
LDA 3, DGNMCC_STACK_PTR
STA 3, 1, 2
LDA 2, @DGNMCC_BASE_PTR
LDA 3, 2, 1 ; Load local variable
ADDZ 2, 3, SKP
0d
LDA 0, 0, 3
MOV 0, 1 ; Post-increment variable
INC 1, 1
STA 1, 0, 3
MOV 0, 3 ; Dereference
LDA 0, 0, 3
JSR @DGNMCC_PUSH ; Push argument 0
LDA 2, DGNMCC_BASE_PTR
LDA 3, 1, 2 ; Setup new lfp and stack adjust
STA 3, 0, 2
MOV 3, 2
LDA 3, 2, 1
ADDZ 2, 3, SKP
1d
STA 3, DGNMCC_STACK_PTR
JMP 6, 1 ; Call extern function
tty_putc
STA 3, -2, 2
LDA 3, -2, 1
LDA 2, DGNMCC_BASE_PTR
JMP 0, 3
JSR -4, 1
LDA 2, DGNMCC_BASE_PTR
LDA 3, 3, 1
ADDZ 2, 3
JMP 0, 3
2d
SUBO 0, 0 ; Default return statement
LDA 3, 0, 2
STA 3, DGNMCC_STACK_PTR
LDA 3, @DGNMCC_POP
STA 3, 0, 2
LDA 3, @DGNMCC_POP
JMP 0, 3
main: LDA 2, DGNMCC_BASE_PTR
LDA 3, 3, 1 ; In line string skip
ADDZ 2, 3
JMP 0, 3
87d
117
165
164
160
165
164
40
164
145
163
164
40
166
141
154
165
145
72
40
0
LDA 0, @DGNMCC_BASE_PTR ; Setup return fp and pc for function
LDA 3, DGNMCC_STACK_PTR
INC 3, 3
STA 3, DGNMCC_STACK_PTR
JSR @DGNMCC_PUSH
LDA 3, DGNMCC_STACK_PTR
STA 3, 1, 2
LDA 0, 2, 1 ; Load pointer to string constant
ADDZ 2, 0, SKP
67d
JSR @DGNMCC_PUSH ; Push argument 0
LDA 3, 1, 2 ; Setup new lfp and stack adjust
STA 3, 0, 2
MOV 3, 2
LDA 3, 2, 1
ADDZ 2, 3, SKP
1d
STA 3, DGNMCC_STACK_PTR
JMP 7, 1 ; Call local function
2d
STA 3, -2, 2
LDA 3, -2, 1
LDA 2, DGNMCC_BASE_PTR
ADDZ 2, 3
JMP 0, 3
JSR -5, 1
LDA 3, 2, 1 ; Load local variable
ADDZ 2, 3, SKP
2d
LDA 0, 0, 3
LDA 3, 2, 1 ; Load local variable
ADDZ 2, 3, SKP
2d
MOV 3, 0
JSR @DGNMCC_PUSH
LDA 0, 2, 1 ; Load numerical constant
JMP 2, 1
4d
LDA 3, @DGNMCC_POP
STA 0, 0, 3
LDA 0, 2, 1 ; Load numerical constant
JMP 2, 1
4d
JSR @DGNMCC_PUSH
LDA 3, 2, 1 ; Load local variable
ADDZ 2, 3, SKP
2d
LDA 0, 0, 3
LDA 1, @DGNMCC_POP
SUBO 1, 0, SNR ; Comparison operation
SUBO 0, 0, SKP
SUBO 0, 0, SKP
INC 0, 0
LDA 2, DGNMCC_BASE_PTR
MOV 0, 0, SZR ; If statement
JMP 5, 1
LDA 3, 3, 1
ADDZ 2, 3
JMP 0, 3
251d
LDA 2, @DGNMCC_BASE_PTR
LDA 3, 2, 1 ; Load local variable
ADDZ 2, 3, SKP
3d
LDA 0, 0, 3
LDA 3, 2, 1 ; Load local variable
ADDZ 2, 3, SKP
2d
LDA 0, 0, 3
MOV 0, 1 ; Post-increment variable
INC 1, 1
STA 1, 0, 3
LDA 2, DGNMCC_BASE_PTR
LDA 0, @DGNMCC_BASE_PTR ; Setup return fp and pc for function
LDA 3, DGNMCC_STACK_PTR
INC 3, 3
STA 3, DGNMCC_STACK_PTR
JSR @DGNMCC_PUSH
LDA 3, DGNMCC_STACK_PTR
STA 3, 1, 2
LDA 2, @DGNMCC_BASE_PTR
LDA 3, 2, 1 ; Load local variable
ADDZ 2, 3, SKP
2d
LDA 0, 0, 3
JSR @DGNMCC_PUSH
LDA 0, 2, 1 ; Load numerical constant
JMP 2, 1
48d
LDA 1, @DGNMCC_POP
ADDZ 1, 0 ; Addition
JSR @DGNMCC_PUSH ; Push argument 0
LDA 2, DGNMCC_BASE_PTR
LDA 3, 1, 2 ; Setup new lfp and stack adjust
STA 3, 0, 2
MOV 3, 2
LDA 3, 2, 1
ADDZ 2, 3, SKP
1d
STA 3, DGNMCC_STACK_PTR
JMP 6, 1 ; Call extern function
tty_putc
STA 3, -2, 2
LDA 3, -2, 1
LDA 2, DGNMCC_BASE_PTR
JMP 0, 3
JSR -4, 1
LDA 2, DGNMCC_BASE_PTR
LDA 0, @DGNMCC_BASE_PTR ; Setup return fp and pc for function
LDA 3, DGNMCC_STACK_PTR
INC 3, 3
STA 3, DGNMCC_STACK_PTR
JSR @DGNMCC_PUSH
LDA 3, DGNMCC_STACK_PTR
STA 3, 1, 2
LDA 0, 2, 1 ; Load numerical constant
JMP 2, 1
13d
JSR @DGNMCC_PUSH ; Push argument 0
LDA 3, 1, 2 ; Setup new lfp and stack adjust
STA 3, 0, 2
MOV 3, 2
LDA 3, 2, 1
ADDZ 2, 3, SKP
1d
STA 3, DGNMCC_STACK_PTR
JMP 6, 1 ; Call extern function
tty_putc
STA 3, -2, 2
LDA 3, -2, 1
LDA 2, DGNMCC_BASE_PTR
JMP 0, 3
JSR -4, 1
LDA 2, DGNMCC_BASE_PTR
LDA 0, @DGNMCC_BASE_PTR ; Setup return fp and pc for function
LDA 3, DGNMCC_STACK_PTR
INC 3, 3
STA 3, DGNMCC_STACK_PTR
JSR @DGNMCC_PUSH
LDA 3, DGNMCC_STACK_PTR
STA 3, 1, 2
LDA 0, 2, 1 ; Load numerical constant
JMP 2, 1
10d
JSR @DGNMCC_PUSH ; Push argument 0
LDA 3, 1, 2 ; Setup new lfp and stack adjust
STA 3, 0, 2
MOV 3, 2
LDA 3, 2, 1
ADDZ 2, 3, SKP
1d
STA 3, DGNMCC_STACK_PTR
JMP 6, 1 ; Call extern function
tty_putc
STA 3, -2, 2
LDA 3, -2, 1
LDA 2, DGNMCC_BASE_PTR
JMP 0, 3
JSR -4, 1
LDA 2, DGNMCC_BASE_PTR
LDA 3, 3, 1 ; Else statement
ADDZ 2, 3
JMP 0, 3
294d
LDA 2, @DGNMCC_BASE_PTR
LDA 3, 2, 1 ; Load local variable
ADDZ 2, 3, SKP
3d
LDA 0, 0, 3
LDA 2, DGNMCC_BASE_PTR
LDA 3, 3, 1 ; In line string skip
ADDZ 2, 3
JMP 0, 3
268d
127
124
106
41
15
12
0
LDA 0, @DGNMCC_BASE_PTR ; Setup return fp and pc for function
LDA 3, DGNMCC_STACK_PTR
INC 3, 3
STA 3, DGNMCC_STACK_PTR
JSR @DGNMCC_PUSH
LDA 3, DGNMCC_STACK_PTR
STA 3, 1, 2
LDA 0, 2, 1 ; Load pointer to string constant
ADDZ 2, 0, SKP
261d
JSR @DGNMCC_PUSH ; Push argument 0
LDA 3, 1, 2 ; Setup new lfp and stack adjust
STA 3, 0, 2
MOV 3, 2
LDA 3, 2, 1
ADDZ 2, 3, SKP
1d
STA 3, DGNMCC_STACK_PTR
JMP 7, 1 ; Call local function
2d
STA 3, -2, 2
LDA 3, -2, 1
LDA 2, DGNMCC_BASE_PTR
ADDZ 2, 3
JMP 0, 3
JSR -5, 1
LDA 2, DGNMCC_BASE_PTR
SUBO 0, 0 ; Default return statement
LDA 3, 0, 2
STA 3, DGNMCC_STACK_PTR
LDA 3, @DGNMCC_POP
STA 3, 0, 2
LDA 3, @DGNMCC_POP
JMP 0, 3
