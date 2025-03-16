**The following assembly syntax is actively being revised. Expect massive changes at any time. Nothing is final.**

# About

This document defines an assembly syntax for the Data General Nova macro assembler. It's intended to be more comprehensible than the original syntax which relied heavily on magic numbers and single letter flags.

# Registers
## Common
Note that the PC is only 15 bits.
```
PC : Program counter
AC0, AC1, AC2, AC3 : Accumulators
IOA, IOB, IOC : I/O Registers
```

## Stack
Note that these registers are only 15 bits. When writing to them bit-0 (MSB) will be discarded. When reading from them bit-0 (MSB) will be zero.
```
SP : Stack Pointer
FP : Frame Pointer
```

## Floating Point
These registers are only applicable if floating point hardware is installed, or a floating point library is available.
```
FPAC : Floating Point Accumulator
FPTM : Floating Point Temp
FPST : Floating Point Status
```

## System
```
IMSK : Interrupt Mask (write-only)
IDEV : Interrupting device code (read-only)

SWR : Front Console Switch Register (read-only)
```

# Special Assembly Labels
The following labels refer to special memory locations on a Nova/Eclipse and may be referenced even if they have not been declared anywhere in a given program.

## Interrupts
The `INT_PTR` label refers to address `1` and should contain a pointer to the interrupt routine.

The `INT_RET` label refers to address `0`. When an interrupt occurs, the current program counter value will be placed at this address prior to the execution of the interrupt routine. This label can be used to resume execution once the interrupt is handled.

For systems equipped with auto-restart functionality, the system will begin execution at address `0`. Therefor, during a power-failure interrupt, a `JMP` instruction to the recovery routine should be placed at `INT_RET`.

## Stack
The `STK_LMT` label refers to address `042 (octal)` or `0x22 (hex)` and should contain the maximum valid stack address. This specific address was chosen to better support the undocument `PSHN` and `SAVN` instructions present on the Nova 4. While you are not required to use this label to hold your stack check value, it is encouraged.

## Trap
The `TRP_PTR` label refers to address `047 (octal)` or `0x26 (hex)` and should contain a pointer to the user-trap routine.

The `TRP_RET` label refers to address `046 (octal)` or `0x27 (hex)`. When a `TRAP` instruction is executed, the address of that instruction will be placed at this address prior to the execution of the trap routine. This label can be used to resume execution once the trap is handled.

# Memory Access
## Load/Store Byte
```
LDB AC#, [AC#]
STB AC#, [AC#]
```

## Load/Store Word
```
LDA AC#, @[<8-Bit Absolute Address>]
STA AC#, @[<8-Bit Absolute Address>]

LDA AC#, @[<Zero Page Label>]
STA AC#, @[<Zero Page Label>]

LDA AC#, @[PC, <(Nothing)|Signed 8-Bit Offset>]
STA AC#, @[PC, <(Nothing)|Signed 8-Bit Offset>]

LDA AC#, @[AC2, <(Nothing)|Signed 8-Bit Offset>]
STA AC#, @[AC2, <(Nothing)|Signed 8-Bit Offset>]

LDA AC#, @[AC3, <(Nothing)|Signed 8-Bit Offset>]
STA AC#, @[AC3, <(Nothing)|Signed 8-Bit Offset>]

LDA AC#, [IO#, 6-Bit Device Code], <(NOTHING)|SET|CLR|PLS>
STA AC#, [IO#, 6-Bit Device Code], <(NOTHING)|SET|CLR|PLS>
```

## Load/Store Long (uses AC0|AC1 as a 32-bit value)
Loads/Stores 2 words from memory into AC0|AC1 as a 32-bit value. This is a useful shorthand for working with `MUL` and `DIV` instructions.

The first word is transfered to/from AC0 (high order bits) and the second word is transfered to/from AC1 (low order bits). This is consistent with the big-endian addressing used by Nova 4 byte operations.

The first word will be stored/loaded from the address specified in the instruction pattern. The second word will be stored/loaded in the subsequent address. For this reason, the specified address offset must NOT be the maximum possible value.

The provided `<8-Bit Absolute Address>` must not be 0xFF
```
LDL [<8-Bit Absolute Address>]
STL [<8-Bit Absolute Address>]

LDL [<Zero Page Label>]
STL [<Zero Page Label>]
```
The provided `<Signed 8-Bit Offset>` must not be 0x7F
```
LDL [PC, <Signed 8-Bit Offset>]
STL [PC, <Signed 8-Bit Offset>]

LDL [AC2, <Signed 8-Bit Offset>]
STL [AC2, <Signed 8-Bit Offset>]

LDL [AC3, <Signed 8-Bit Offset>]
STL [AC3, <Signed 8-Bit Offset>]
```

## Load/Store Float
```
LDF FPAC, [AC#]
STF FPAC, [AC#]
```

## Load/Store Double
```
LDD FPAC, [AC#]
STD FPAC, [AC#]
```

# Flow Control
## Jump

```
JMP @[<8-Bit Absolute Address>]
JMP @[<Zero Page Label>]
JMP @[PC, <Signed 8-Bit Offset>]
JMP @[AC2, <Signed 8-Bit Offset>]
JMP @[AC3, <Signed 8-Bit Offset>]
JMP <16-Bit Absolute Address>
JMP <LABEL>
```
For `JMP <16-Bit Absolute Address>` and `JMP <LABEL>` the address will be placed in the word immediately after the `JMP` instruction.

## Jump and Link

Place next address into AC3 then preform an unconditional jump to the specified address
```
JSR @[<8-Bit Absolute Address>]
JSR @[<Zero Page Label>]
JSR @[PC, <Signed 8-Bit Offset>]
JSR @[AC2, <Signed 8-Bit Offset>]
JSR @[AC3, <Signed 8-Bit Offset>]
JSR <16-Bit Absolute Address>
JSR <LABEL>
```
For `JSR <16-Bit Absolute Address>` and `JSR <LABEL>` the address will be placed in the word immediately after the `JSR` instruction.

## Modify and Test
Increment value at indicated address by 1 and skip next instruction if result is zero
```
ISZ @[<8-Bit Absolute Address>]
ISZ @[<Zero Page Label>]
ISZ @[PC, <Signed 8-Bit Offset>]
ISZ @[AC2, <Signed 8-Bit Offset>]
ISZ @[AC3, <Signed 8-Bit Offset>]
```
Decrement value at indicated address by 1 and skip next instruction if result is zero
```
DSZ @[<8-Bit Absolute Address>]
DSZ @[<Zero Page Label>]
DSZ @[PC, <Signed 8-Bit Offset>]
DSZ @[AC2, <Signed 8-Bit Offset>]
DSZ @[AC3, <Signed 8-Bit Offset>]
```

# Arithmetic & Logic Flags
## Carry Control
```
SET : Set Carry to one before operation
CLR : Set Carry to zero before operation
INV : Invert Carry before operation
```
## Shift Control
```
SHL : Shift result left one bit (MSB goes into Carry)
SHR : Shift result right one bit (LSB goes into Carry)
SWP : Swap the upper and lower byte of the result (Carry unaffected)
```
## Skip Condition
If `#` is L (Load) : Place result in destination accumulator then preform skip

If `#` is T (Test) : Preform skip without modifying the destination accumulator

```
(Nothing) : Place result in destination
LSKP : Place result in destination accumulator and skip the next instruction.
TSKP : Skip the next instruction without modifying the destination accumulator. This is effectively a jump instruction.

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
MOV <(NOTHING)|SET|CLR|INV>, ACD#, ACS#, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

## Accumulator to/from FP Accumulator
Place bits 1-7 of Accumulator into bits 1-7 (exponent) of FP Accumulator
```
FSPUT #AC, FPAC
FDPUT #AC, FPAC
```

Place the most significant 16 bits of FP Accumulator into Accumulator
```
FSGET AC#, FPAC
FDGET AC#, FPAC
```

## FP Accumulator to FP Accumulator
```
FSMOV FPAC, FPTM
FSMOV FPTM, FPAC
```

# Empty
## Accumulator
Set all bits of AC# to zero

This operation does not affect the carry bit.

Order of operations: Modify Carry --> Preform Operation --> Shift Result --> Evaluate Skip Condition
```
EMT <(NOTHING)|SET|CLR|INV>, ACD#, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>

Decodes as (invert carry selection behavior as this will toggle the carry bit):
SUB <(NOTHING)|SET|CLR|INV>, ACD#, ACD# <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

## FP Accumulator
Set all bits of FPAD to zero
```
EMT FPAC
```
# Fill
## Accumulator
Set all bits of AC# to one

This operation does not affect the carry bit.

Order of operations: Modify Carry --> Preform Operation --> Shift Result --> Evaluate Skip Condition
```
FIL <(NOTHING)|SET|CLR|INV>, ACD#, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>

Decodes as (invert cary selection behavior as this will toggle the carry bit):
ADC <(NOTHING)|SET|CLR|INV>, ACD#, ACD# <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```
# Addition
## Add Accumulator to Accumulator
Add the unsigned value of source to destination. If this operation produces a carry value of 1, then invert the Carry bit.

Order of operations: Modify Carry --> Preform Operation --> Shift Result --> Evaluate Skip Condition
```
ADD <(NOTHING)|SET|CLR|INV>, ACD#, ACS#, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

## Add value in memory to FP Accumulator
Add two words in memory to the floating point accumulator
```
FSADD [AC#]
```
Add four words in memory to the floating point accumulator
```
FDADD [AC#]
```

## Add FP Accumulator to FP Accumulator
Add FP Temp to FP Accumulator
```
FSADD TEMP
FDADD TEMP
```

# Subtract
## Subtract Accumulator from Accumulator
Subtract the unsigned value of the source from the destination. If this operation produces a carry value of 1, then invert the Carry bit.

Order of operations: Modify Carry --> Preform Operation --> Shift Result --> Evaluate Skip Condition
```
SUB <(NOTHING)|SET|CLR|INV>, ACD#, ACS#, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

## Subtract value in memory from FP Accumulator
Subtract two words in memory from the floating point accumulator
```
FSSUB [AC#]
```
Subtract four words in memory from the floating point accumulator
```
FDSUB [AC#]
```

## Subtract FP Accumulator from FP Accumulator
```
FSSUB FPTM
FDSUB FPTM
```

# Multiply
## Multiply Accumulator
Unsigned multiplication
```
MULU
```
Signed multiplication
```
MULS
```

## Multiply value in memory with FP Accumulator
Multiply two words in memory with the floating point accumulator
```
FSMUL [AC#]
```
Multiply four words in memory with the floating point accumulator
```
FDMUL [AC#]
```

## Multiply FP Accumulator with FP Accumulator
```
FSMUL FPTM
FDMUL FPTM
```

# Division
## Divide Accumulator by Accumulator
Unsigned division
```
DIVU
```
Signed division
```
DIVS
```

## Divide value in memory with FP Accumulator
Divide floating point accumulator by two words in memory
```
FSDIV [AC#]
```
Divide floating point accumulator by four words in memory
```
FDDIV [AC#]
```

## Divide FP Accumulator by FP Accumulator
Divide first argument by second argument
```
FSDIV FPTM
FDDIB FPTM
```

# Negation
## Negate Accumulator
Take the value of the source register as two's compliment and negate it. If the source register was zero, then invert the carry bit.

Order of operations: Modify Carry --> Preform Operation --> Shift Result --> Evaluate Skip Condition
```
NEG <(NOTHING)|SET|CLR|INV>, ACD#, ACS#, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

## Negate FP Accumulator
Invert the sign bit of the floating point accumulator
```
FNEG
```

# Absolute Value
## Accumulator
Place the unsigned absolute value of ACS# into ACD#

This operation inverts the carry bit if ACS# was negative

Order of operations: Modify Carry --> Preform Operation --> Shift Result --> Evaluate Skip Condition
```
ABS <(NOTHING)|SET|CLR|INV>, ACD#, ACS#, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>

Decodes as:
MOV <(NOTHING)|SET|CLR|INV>, ACS, ACS, SHL, TCZ
NEG INV, ACD, ACS <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

## FP Accumulator Absolute
Set the most significant bit (sign bit) to zero
```
FABS
```

# Scale
## Scale FP Accumulator
Scale the FP Accumulator according to the contents of bits 1-7 of the specified accumulator
```
FSCL AC#
```

# Normalize
## Normalize Accumulator
Normalize `AC0|AC1`
```
NORM
```
## Normalize FP Accumulator
Normalize the Floating Point Accumulator
```
FNORM
```

# Bitwise Operations
## Not
Invert all the bits in source. Does not affect the carry bit. For legacy purpouses, `COM` is a valid alias for `NOT`

Order of operations: Modify Carry --> Preform Operation --> Shift Result --> Evaluate Skip Condition

```
NOT <(NOTHING)|SET|CLR|INV>, ACD#, ACS#, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

## And
Compute the bitwise and of the source and destination. Does not affect the carry bit.

Order of operations: Modify Carry --> Preform Operation --> Shift Result --> Evaluate Skip Condition

```
AND <(NOTHING)|SET|CLR|INV>, ACD#, ACS#, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

## Or
Compute the bitwise or of the source and destination. Alters the carry bit if `ACS# < ACD#`

```
OR ACD#, ACS#

Decodes as:
NOT ACS, ACS
AND ACD, ACS
ADC ACD, ACS ; Compliments carry bit if ACS < ACD
NOT ACS, ACS
```
## Xor
Compute the bitwise xor of the source and destination. Destroys the carry bit.

```
XOR ACD#, ACS#, ACT#

Decodes as:
MOV ACT, ACD
AND CLR, ACT, ACS, SHL ; Destroys carry
ADD ACD, ACS
SUB ACD, ACT
```

# Increment
## Increment Accumulator
Add one to the source. If the value in source was 0xFFFF, invert the carry bit.

Order of operations: Modify Carry --> Preform Operation --> Shift Result --> Evaluate Skip Condition

```
INC <(NOTHING)|SET|CLR|INV>, ACD#, ACS#, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

# Decrement
## Decrement Accumulator
Subtract one from the source. If the value in source was 0x0000, invert the carry bit.

Order of operations: Modify Carry --> Preform Operation --> Shift Result --> Evaluate Skip Condition

```
DEC <(NOTHING)|SET|CLR|INV>, ACD#, ACS#, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>

Decodes as:
NEG <(NOTHING)|SET|CLR|INV>, ACD, ACS
INV ACD, ACD, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

# Add Complement
Invert all the bits in source and add to destination. If the value in source is less than the value in destination, invert the carry bit.

Order of operations: Modify Carry --> Preform Operation --> Shift Result --> Evaluate Skip Condition

```
ADC <(NOTHING)|SET|CLR|INV>, ACD#, ACS#, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

# Floating Point Instructions

TODO

# Stack Instructions
A stack overflow interrupt is generated every time a `PSH` or `SAV` results in a word being saved to an address which is an integral multiple of `256 (decimal)` or `0xFF (hex)`. The interrupt handler should check to see if the stack pointer exceeds the value present at the `STK_LMT` label and act accordingly.

## Push
Pushes the contents of the specified accumulator to onto the stack and increments the stack pointer by one.
```
PSH AC#
```

## Pop
Pops a word off the stack and places it into the specified accumulator and decrements the stack pointer by one.
```
POP AC#
```

## Save
Pushes a return block onto the stack. This is typically called after executing a `JSR` instruction to save the processor state.

```
SAV
```

The return block is strucutred as follows:

| Order Pushed | Contents |
| ------------ | -------- |
| 1 | AC0 |
| 2 | AC1 |
| 3 | AC2 |
| 4 | FP |
| 5 | Carry + AC3 |

For the 4th word, bit-0 (MSB) is set to the Carry bit and bits 1-15 are set to the contents of AC3 bits 1-15. The MSB of AC3 is not saved.

After the return block is pushed the stack pointer is placed into the frame pointer.

## Return
Pops a return block off of the stack and jumps to the address saved within. This is called at the end of a function to restore the processor state.

```
RET
```

Before the return block is poped off the stack, the contents of the frame pointer are placed into the stack pointer. That being said, ensure that the frame pointer points to a valid return block before using this instruction.

The contents of the return block are placed into the following locations:

| Order Popped | Destination |
| ------------ | ----------- |
| 1 | Carry + PC |
| 2 | FP, AC3 |
| 3 | AC2 |
| 4 | AC1 |
| 5 | AC0 | 

For the 1st word, bit-0 (MSB) is placed into the Carry bit and the remaining bits 1-15 are placed into the PC.

The 2nd word is deposited into both the FP and AC3.

After all words are popped, execution will resume from the new value placed into the program counter.

# I/O Instructions
None of the following I/O instructions will transfer data to or from an accumulator, therefore you are not required to specify one. Regardless, the instruction format does allow for you to specify an accumulator so an optional field is provided should you choose to do so. Otherwise, AC0 will be specified for you
## Control
Set the Busy Flag & Clear the Done Flag
```
IO SET <6-Bit Device Code>, <Nothing|AC0|AC1|AC2|AC3>
```
Clear both Busy & Done flags
```
IO CLR <6-Bit Device Code>, <Nothing|AC0|AC1|AC2|AC3>
```
Send an Pulse to the device
```
IO PLS <6-Bit Device Code>, <Nothing|AC0|AC1|AC2|AC3>
```
## Conditional
Skip conditions
```
SBZ : Skip if the busy flag is zero
SBN : Skip if the busy flag is non-zero
SDZ : Skip if the done flag is zero
SDN : Skip if the done flag is non-zero
```
Skip next instruction if condition is true
```
IO <SBZ|SBN|SDZ|SDN> <6-Bit Device Code>, <Nothing|AC0|AC1|AC2|AC3>
```


# GET & PUT Instructions
These instructions are used to transfer between an accumulator and a system register. Some additional arguments may be allowed/required depending on the type transfer.

```  
GET AC#, <Register> : Transfer from Register to AC#
PUT AC#, <Register> : Transfer from AC# to Register
```

## System Control Registers

Transfer AC# to the interrupt mask. Do not use while interrupts are enabled. Modifies the interrupt enable flag as desired.
```
PUT AC#, IMSK, <Nothing|INTEN|INTDS>
```

Transfer the 6-bit device code of the interrupting device to AC#. Modifies the interrupt enable flag as desired.
```
GET AC#, IDEV, <Nothing|INTEN|INTDS>
```

Transfer the contents of the front panel's switches to AC#. Modifies the interrupt enable flag as desired.
```
GET #AC, SWR, <Nothing|INTEN|INTDS>
```

## Stack Pointer
```
GET AC#, SP
PUT AC#, SP
```

## Frame Pointer
```
GET AC#, FP
PUT AC#, FP
```

## Floating Point Status
```
GET AC#, FPST
PUT AC#, FPST
```

# System Instructions
## Reset
Send a reset pulse to all I/O devices. Modifies the interrupt enable flag as desired.
```
IO RST, <Nothing|INTEN|INTDS>
```

## Interrupt Control
Toggle the interrupt enable flag.
```
INTEN : Enable Interrupts
INTDS : Disable Interrupts
```

## Interrupt Testing
Skip next instruction based on interrupt flag.
```
INTSZ : Skip if interrupts are disabled
INTSN : Skip if interrupts are enabled
```

## Power-Fail Testing
Skip next instruction based on power-fail flag
```
PWRSZ : Skip if power is okay
PWRSN : Skip if power has failed
```

## Halt
Immediately suspend CPU execution. Modifies the interrupt enable flag as desired.
```
HALT <Nothing|INTEN|INTDS>
```

## Trap
Triggers a system trap interrupt
```
TRAP <11-Bit Value>
TRAP AC#, <9-Bit Value>
TRAP AC#, AC#, <7-Bit Value>
```
