# About
Unless stated otherwise, the left most argument is almost always the destination!

# Registers
## Common
```
ZP : Zero page
PC : Program counter
AC[0-3] : Accumulators
ACL : The 32bit union of AC0 & AC1 with AC0 forming the high order bits
IO[A-C] : I/O Registers
```

## Stack
```
SP : Stack Pointer
FP : Frame Pointer
```

## Floating Point
The single accumulators are the upper half of the double accumulators, so they will interfere with each other
```
FPAD : Floating Point Accumulator (Double)
FPTD : Floating Point Temp (Double)

FPAS : Floating Point Accumulator (Single)
FPTS : Floating Point Temp (Single)

FPST : Floating Point Status
```

## System
```
IMSK : Interupt Mask
IDEV : Interupting device code

SWR : Front Console Switch Register
```

# Memory Access
## Load/Store Byte
```
LDB AC#, [AC#]
STB AC#, [AC#]
```

## Load/Store Word
The `@` is optional and implies that indirection should occur
```
LD AC#, @[ZP, <(Nothing)|8-Bit Absolute Address>]
ST AC#, @[ZP, <(Nothing)|8-Bit Absolute Address>]

LD AC#, @[PC, <(Nothing)|Signed 8-Bit Offset>]
ST AC#, @[PC, <(Nothing)|Signed 8-Bit Offset>]

LD AC#, @[AC2, <(Nothing)|Signed 8-Bit Offset>]
ST AC#, @[AC2, <(Nothing)|Signed 8-Bit Offset>]

LD AC#, @[AC3, <(Nothing)|Signed 8-Bit Offset>]
ST AC#, @[AC3, <(Nothing)|Signed 8-Bit Offset>]

LD AC#, [IO#, 6-Bit Device Code], <(NOTHING)|SET|CLR|PLS>
ST AC#, [IO#, 6-Bit Device Code], <(NOTHING)|SET|CLR|PLS>
```

## Load/Store Long (uses AC0|AC1 as a 32bit value)
The provided `<8-Bit Absolute Address>` must not be 0xFF
```
LD ACL, [ZP, <8-Bit Absolute Address>]
ST ACL, [ZP, <8-Bit Absolute Address>]
```
The provided `<Signed 8-Bit Offset>` must not be 0x7F
```
LD ACL, [PC, <Signed 8-Bit Offset>]
ST ACL, [PC, <Signed 8-Bit Offset>]

LD ACL, [AC2, <Signed 8-Bit Offset>]
ST ACL, [AC2, <Signed 8-Bit Offset>]

LD ACL, [AC3, <Signed 8-Bit Offset>]
ST ACL, [AC3, <Signed 8-Bit Offset>]
```

## Load/Store Float
```
LD FPAS, [AC#]
ST FPAS, [AC#]

LD FPAD, [AC#]
ST FPAD, [AC#]
```

# Flow Control
## Jump
Unconditional jump
```
JMP @[ZP, <8-Bit Absolute Address>]
JMP @[PC, <Signed 8-Bit Offset>]
JMP @[AC2, <Signed 8-Bit Offset>]
JMP @[AC3, <Signed 8-Bit Offset>]
```
Jump and link
Place next address into AC3 then preform an unconditional jump to the specified address
```
JMP @[ZP, <8-Bit Absolute Address>]
JMP @[PC, <Signed 8-Bit Offset>]
JMP @[AC2, <Signed 8-Bit Offset>]
JMP @[AC3, <Signed 8-Bit Offset>]
```

## Modify and Test
Increment by 1 and skip next instruction if result is zero
```
ISZ @[ZP, <8-Bit Absolute Address>]
ISZ @[PC, <Signed 8-Bit Offset>]
ISZ @[AC2, <Signed 8-Bit Offset>]
ISZ @[AC3, <Signed 8-Bit Offset>]
```
Decrement by 1 and skip next instruction if result is zero
```
DSZ @[ZP, <8-Bit Absolute Address>]
DSZ @[PC, <Signed 8-Bit Offset>]
DSZ @[AC2, <Signed 8-Bit Offset>]
DSZ @[AC3, <Signed 8-Bit Offset>]
```

# Arithmetic & Logic Flags
## Carry Control
```
SET : Set Carry to one before operation
CLR : Set Carry to zero before operation
FLP : Invert Carry before operation
```
## Shift Control
```
SHL : Shift result left one bit (MSB goes into Carry)
SHR : Shift result right one bit (LSB goes into Carry)
SWP : Swap the upper and lower byte of the result (Carry unaffected)
```
## Skip Condition
If `#` is L (load) : Place result in destination then preform skip

If `#` is T (test) : DO NOT Place result in destination then preform skip

```
(Nothing) : Place result in destination
SKP : Place result in destination and skip the next instruction

#CZ : Skip if the Carry bit is zero
#CN : Skip if the Carry bit is non-zero
#RZ : Skip if the Result is zero
#RN : Skip if the Result is non-zero
#EZ : Skip if either the Carry or Result (or both) are zero
#BN : Skip if both the Carry and Result are non-zero
```
# Move
## Accumulator to Accumulator
This operation does not affect the carry bit.

Order of operations: Modify Carry --> Preform Operation --> Shift Result --> Evaluate Skip Condition

```
MOV <(NOTHING)|SET|CLR|FLP>, AC#, AC#, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|SKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

## Accumulator to/from Stack Pointer
```
MOV AC#, SP
MOV SP, AC#
```

## Accumulator to/from Frame Pointer
```
MOV AC#, FP
MOV FP, AC#
```
## Accumulator to Interrupt Mask
```
MOV IMSK, AC#
```

## Interrupting Device Code to Accumulator
```
MOV AC#, IDEV
```

## Front Console Switch Register to Accumulator
```
MOV AC#, SWR
```

## Accumulator to/from FP Accumulator
Place bits 1-7 of Accumulator into bits 1-7 (exponent) of FP Accumulator
```
MOV FPAS, AC#
MOV FPAD, AC#
```

Place the most significant 16 bits of FP Accumulator into Accumulator
```
MOV AC#, FPAS
MOV AC#, FPAD
```
## FP Accumulator to FP Accumulator
```
MOV FPAS, FPTS
MOV FPTS, FPAS

MOV FPAD, FPTD
MOV FPTD, FPAD
```
## FP Status to/from Accumulator
```
MOV AC#, FPST
MOV FPST, AC#
```
# Clear
## Accumulator 
This operation does not affect the carry bit.

Order of operations: Modify Carry --> Preform Operation --> Shift Result --> Evaluate Skip Condition
```
CLR <(NOTHING)|SET|CLR|FLP>, AC#, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|SKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

## FP Accumulator
```
CLR FPAD
```
# Addition
## Add Accumulator to Accumulator
Add the unsigned value of source to destination. If this operation produces a carry value of 1, then invert the Carry bit.

Order of operations: Modify Carry --> Preform Operation --> Shift Result --> Evaluate Skip Condition
```
ADD <(NOTHING)|SET|CLR|FLP>, AC#, AC#, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|SKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

## Add value in memory to FP Accumulator
Add two words in memory to the floating point accumulator
```
ADD FPAS, [AC#]
```
Add four words in memory to the floating point accumulator
```
ADD FPAD, [AC#]
```

## Add FP Accumulator to FP Accumulator
Add FP Temp to FP Accumulator
```
ADD FPAS, FPTS
ADD FPAD, FPTD
```

# Subtract
## Subtract Accumulator from Accumulator
Subtract the unsigned value of the source from the destination. If this operation produces a carry value of 1, then invert the Carry bit.

Order of operations: Modify Carry --> Preform Operation --> Shift Result --> Evaluate Skip Condition
```
SUB <(NOTHING)|SET|CLR|FLP>, AC#, AC#, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|SKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

## Subtract value in memory from FP Accumulator
Subtract two words in memory from the floating point accumulator
```
SUB FPAS, [AC#]
```
Subtract four words in memory from the floating point accumulator
```
SUB FPAD, [AC#]
```

## Subtract FP Accumulator from FP Accumulator
```
SUB FPAS, FPTS
SUB FPAD, FPTD
```

# Multiply
## Multiply Accumulator
Unsigned multiplication
```
MUL ACL
```
Signed multiplication
```
MULS ACL
```

## Multiply value in memory with FP Accumulator
Multiply two words in memory with the floating point accumulator
```
MUL FPAS, [AC#]
```
Multiply four words in memory with the floating point accumulator
```
MUL FPAD, [AC#]
```

## Multiply FP Accumulator with FP Accumulator
```
MUL FPAS, FPTS
MUL FPAD, FPTD
```

# Division
## Divide Accumulator by Accumulator
Unsigned division
```
DIV ACL
```
Signed division
```
DIVS ACL
```

## Divide value in memory with FP Accumulator
Divide floating point accumulator by two words in memory
```
DIV FPAS, [AC#]
```
Divide floating point accumulator by four words in memory
```
DIV FPAD, [AC#]
```

## Divide FP Accumulator by FP Accumulator
Divide first argument by second argument
```
DIV FPAS, FPTS
DIV FPAD, FPTD
```

# Negation
## Negate Accumulator
Take the value of the source register as two's compliment and negate it. If the source register was zero, then invert the carry bit.

Order of operations: Modify Carry --> Preform Operation --> Shift Result --> Evaluate Skip Condition
```
NEG <(NOTHING)|SET|CLR|FLP>, AC#, AC#, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|SKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

## Negate FP Accumulator
Invert the sign bit of the floating point accumulator
```
NEG FPAS
NEG FPAD
```

# Absolute Value
## FP Accumulator Absolute
Set the most significant bit (sign bit) to zero
```
ABS FPAS
ABS FPAD
```

# Scale
## Scale FP Accumulator
Scale the FP Accumulator
```
SCLE FPAS
SCLE FPAD
```

# Normalize
## Normalize Accumulator
Normalize ACL
```
NORM ACL
```
## Normalize FP Accumulator
Normalize the Floating Point Accumulator
```
NORM FPAS
NORM FPAD
```

# Bitwise Operations
## Not
Invert all the bits in source. Does not affect the carry bit.

Order of operations: Modify Carry --> Preform Operation --> Shift Result --> Evaluate Skip Condition

```
NOT <(NOTHING)|SET|CLR|FLP>, AC#, AC#, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|SKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```
## And
Compute the bitwise and of the source and destination. Does not affect the carry bit.

Order of operations: Modify Carry --> Preform Operation --> Shift Result --> Evaluate Skip Condition

```
AND <(NOTHING)|SET|CLR|FLP>, AC#, AC#, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|SKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

## Or
Compute the bitwise or of the source and destination. Does not affect the carry bit.

Order of operations: Modify Carry --> Preform Operation --> Shift Result --> Evaluate Skip Condition

```
OR AC#, AC#
```
## Xor
Compute the bitwise xor of the source and destination. Does not affect the carry bit.

Order of operations: Modify Carry --> Preform Operation --> Shift Result --> Evaluate Skip Condition

```
XOR AC#, AC#
```

# Increment
## Increment Accumulator
Add one to the source. If the value in source was 0xFFFF, invert the carry bit.

Order of operations: Modify Carry --> Preform Operation --> Shift Result --> Evaluate Skip Condition

```
INC <(NOTHING)|SET|CLR|FLP>, AC#, AC#, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|SKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

# Add Complement
Invert all the bits in source and add to destination. If the value in source is less than the valuei destination, invert the carry bit.

Order of operations: Modify Carry --> Preform Operation --> Shift Result --> Evaluate Skip Condition

```
INC <(NOTHING)|SET|CLR|FLP>, AC#, AC#, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|SKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

# I/O Instructions
## Control
Set the Busy Flag & Clear the Done Flag
```
IOSET <6-Bit Device Code>
```
Clear both Busy & Done flags
```
IOCLR <6-Bit Device Code>
```
Send an Pulse to the device
```
IOPLS <6-Bit Device Code>
```

## Conditional
Skip conditions
```
BZ : Skip if the busy flag is zero
BN : Skip if the busy flag is non-zero
DZ : Skip if the done flag is zero
DN : Skip if the done flag is non-zero
```
Skip next instruction if condition is true
```
IOSKP <6-Bit Device Code>, <BZ|BN|DZ|DN>
```

# CPU Control
## I/O Control
Enable Interrupt
```
INTEN
```
Disable Interrupt
```
INTDS
```
I/O Reset
```
IORST
```
## Flow Control
Stop execution until resumed by the system console
```
HALT
```
Triggers a system trap interrupt
```
TRAP 11-Bit Value
TRAP AC#, AC#, 7-Bit Value
```
