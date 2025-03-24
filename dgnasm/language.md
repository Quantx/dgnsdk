**The following assembly syntax is actively being revised. Expect massive changes at any time. Nothing is final.**

# About

This document defines an assembly syntax for the Data General Nova macro assembler. It's intended to be more comprehensible than the original syntax which relied heavily on magic numbers and single letter flags.

# Registers
## Common
Note that the PC, SP and FP registers are only 15 bits. When writing to them bit-0 (MSB) will be discarded. When reading from them bit-0 (MSB) will be zero.

```
PC : Program counter
A0, A1, A2, A3 : Accumulators
SP : Stack Pointer
FP : Frame Pointer

ACS : Used to denote a source accumulator
ACD : Used to denote a destination accumulator
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

# Labels

# System Defined Labels
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
Transfers a byte from the least significant 8 bits of an accumulator (left argument) into the memory address contained in the (right argument).
```
LDB ACD, AC
STB ACS, AC
```

## Load/Store Word
Transfers a byte between accumulator and a specified memory location. If the `@` modifier is specified, then the specified memory location is used as a pointer to the actual location. If the pointer itself has it's MSB set to one (by specifying the `@` prefix on a label) then the value pointed to is also assumed to be a pointer. This check is repeated forever until a value is found with it's MSB set to zero which is considered to be the final pointer.

```
LD[Nothing|@] ACD, <8-Bit Absolute Address>
ST[Nothing|@] ACS, <8-Bit Absolute Address>
	
LD[Nothing|@] ACD, <Zero Page Label>
ST[Nothing|@] ACS, <Zero Page Label>

LD[Nothing|@] ACD, PC ± <Signed 8-Bit Offset>
ST[Nothing|@] ACS, PC ± <Signed 8-Bit Offset>

LD[Nothing|@] ACD, A2 ± <Signed 8-Bit Offset>
ST[Nothing|@] ACS, A2 ± <Signed 8-Bit Offset>

LD[Nothing|@] ACD, A3 ± <Signed 8-Bit Offset>
ST[Nothing|@] ACS, A3 ± <Signed 8-Bit Offset>
```

## Load/Store Double (uses AC0|AC1 as a 32-bit value)
Loads/Stores 2 words from memory into AC0|AC1 as a 32-bit value. This is a useful shorthand for working with `MUL[U|S]` and `DIV[U|S]` instructions.

The first word is transfered to/from AC0 (high order bits) and the second word is transfered to/from AC1 (low order bits). This is consistent with the big-endian addressing used by Nova 4 byte operations.

The first word will be stored/loaded from the address specified in the instruction pattern. The second word will be stored/loaded in the subsequent address. For this reason, the specified address offset must NOT be the maximum possible value.

The provided `<8-Bit Absolute Address>` must not be 0xFF
```
LDD <8-Bit Absolute Address>
STD <8-Bit Absolute Address>

LDD <Zero Page Label>
STD <Zero Page Label>
```
The provided `<Signed 8-Bit Offset>` must not be 0x7F
```
LDD PC ± <Signed 8-Bit Offset>
STD PC ± <Signed 8-Bit Offset>

LDD AC2 ± <Signed 8-Bit Offset>
STD AC2 ± <Signed 8-Bit Offset>

LDD AC3 ± <Signed 8-Bit Offset>
STD AC3 ± <Signed 8-Bit Offset>
```

# Flow Control
## Jump
Jump to the specified address
```
JMP[Nothing|@] <8-Bit Absolute Address>
JMP[Nothing|@] <Zero Page Label>

JMP[Nothing|@] PC ± <Signed 8-Bit Offset>

JMP[Nothing|@] AC2 ± <Signed 8-Bit Offset>

JMP[Nothing|@] AC3 ± <Signed 8-Bit Offset>
```

## Jump and Link
Place next address into AC3 then preform an unconditional jump to the specified address
```
JSR[Nothing|@] <8-Bit Absolute Address>
JSR[Nothing|@] <Zero Page Label>

JSR[Nothing|@] PC ± <Signed 8-Bit Offset>

JSR[Nothing|@] AC2 ± <Signed 8-Bit Offset>

JSR[Nothing|@] AC3 ± <Signed 8-Bit Offset>
```

## Modify and Test
Increment value at indicated address by 1 and skip next instruction if result is zero
```
ISZ[Nothing|@] <8-Bit Absolute Address>
ISZ[Nothing|@] <Zero Page Label>

ISZ[Nothing|@] PC ± <Signed 8-Bit Offset>

ISZ[Nothing|@] AC2 ± <Signed 8-Bit Offset>

ISZ[Nothing|@] AC3 ± <Signed 8-Bit Offset>
```

Decrement value at indicated address by 1 and skip next instruction if result is zero
```
DSZ[Nothing|@] <8-Bit Absolute Address>
DSZ[Nothing|@] <Zero Page Label>

DSZ[Nothing|@] PC ± <Signed 8-Bit Offset>

DSZ[Nothing|@] AC2 ± <Signed 8-Bit Offset>

DSZ[Nothing|@] AC3 ± <Signed 8-Bit Offset>
```

# Extended Immediate Addressing
To simplify addressing memory locations outside zero page, extended-immediate mode macros are available for all standard memory instructions. These macros attempt to place a constant containing the memory location within a -128 to +127 range of the instruction itself. This is accomplished by inserting the constant after an ALU instruction that does not have a skip condition specified. The ALU instruction will be modified to skip over the inserted constant.

If no suitable ALU instruction can be located within the -128 to +127 region, the following instructions will be emitted instead.
```  
<IMM_INST> PC + 2
JMP PC + 2
<LABEL|16-Bit Constant>
```

## Load/Store

Transfer the contents of the memory address specified by the label or constant to/from the accumulator.
```
LDI ACD, <LABEL>
LDI ACD, <16-Bit Constant>

STI ACS, <LABEL>
STI ACS, <16-Bit Constant>
```

## Jump
Jump to the address specified by the label or constant.
```
JMPI <LABEL>
JMPI <16-Bit Constant>
```

## Jump and Link
Place next address into AC3 then preform an unconditional jump to the address specified by the label or constant.
```
JSRI <LABEL>
JSRI <16-Bit Constant>
```

## Modify and Test
Increment value at the address specified by the label or constant by 1 and skip next instruction if result is zero.
```
ISZI <LABEL>
ISZI <16-Bit Constant>
```

Decrement value at the address specified by the label or constant by 1 and skip next instruction if result is zero.
```
DSZI <LABEL>
DSZI <16-Bit Constant>
```

## Move
Places the value of the label or constant into the specified accumulator.
```
MVI ACD, <LABEL>
MVI ACD, <16-Bit Constant>
```

# Arithmetic & Logic Flags
## Carry Modifiers
```
Z = Set Carry to 0 before operation
O = Set Carry to 1 before operation
C = Complement (invert) Carry before operation
```
## Shift Modifiers
```
L : 17-bit Rotate result left one bit with (MSB goes into Carry)
R : 17-bit Rotate result right one bit (LSB goes into Carry)
S : Swap the upper and lower byte of the result (Carry unaffected)
```
## Skip Condition
If `#` is L (Load) : Place result in destination accumulator then preform skip

If `#` is T (Test) : Preform skip without modifying the destination accumulator

```
(Nothing) : Place result in destination
SKP : Place result in destination accumulator and skip the next instruction.

#CZ : Skip if the Carry bit is zero
#CN : Skip if the Carry bit is non-zero
#RZ : Skip if the Result is zero
#RN : Skip if the Result is non-zero
#EZ : Skip if either the Carry or Result (or both) are zero
#BN : Skip if both the Carry and Result are non-zero
```
## Move
This operation does not affect the carry bit.

Order of operations: Modify Carry → Preform Operation → Shift Result → Evaluate Skip Condition

```
MOV <(NOTHING)|SET|CLR|INV>, ACD, ACS, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

## Empty

Set all bits of AC to zero

This operation does not affect the carry bit.

Order of operations: Modify Carry → Preform Operation → Shift Result → Evaluate Skip Condition
```
EMT <(NOTHING)|SET|CLR|INV>, ACD, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>

Decodes as (invert carry selection behavior as this will toggle the carry bit):
SUB <(NOTHING)|SET|CLR|INV>, ACD, ACD <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

## Fill
Set all bits of AC to one

This operation does not affect the carry bit.

Order of operations: Modify Carry → Preform Operation → Shift Result → Evaluate Skip Condition
```
FIL <(NOTHING)|SET|CLR|INV>, ACD, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>

Decodes as (invert cary selection behavior as this will toggle the carry bit):
ADC <(NOTHING)|SET|CLR|INV>, ACD, ACD <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```
## Addition
Add the unsigned value of source to destination. If this operation produces a carry value of 1, then invert the Carry bit.

Order of operations: Modify Carry → Preform Operation → Shift Result → Evaluate Skip Condition
```
ADD <(NOTHING)|SET|CLR|INV>, ACD, ACS, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

## Subtract
Subtract the unsigned value of the source from the destination. If this operation produces a carry value of 1, then invert the Carry bit.

Order of operations: Modify Carry → Preform Operation → Shift Result → Evaluate Skip Condition
```
SUB <(NOTHING)|SET|CLR|INV>, ACD, ACS, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

## Multiply
Unsigned multiplication
```
MULU
```
Signed multiplication
```
MULS
```

## Division
Unsigned division
```
DIVU
```
Signed division
```
DIVS
```

## Negation
Take the value of the source register as two's compliment and negate it. If the source register was zero, then invert the carry bit.

Order of operations: Modify Carry → Preform Operation → Shift Result → Evaluate Skip Condition
```
NEG <(NOTHING)|SET|CLR|INV>, ACD, ACS, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

## Absolute Value
Place the unsigned absolute value of ACS into ACD

This operation inverts the carry bit if ACS was negative

Order of operations: Modify Carry → Preform Operation → Shift Result → Evaluate Skip Condition
```
ABS <(NOTHING)|SET|CLR|INV>, ACD, ACS, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>

Decodes as:
MOV <(NOTHING)|SET|CLR|INV>, ACS, ACS, SHL, TCZ
NEG INV, ACD, ACS <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

# Bitwise Operations
## Not
Invert all the bits in source. Does not affect the carry bit. For legacy purpouses, `COM` is a valid alias for `NOT`

Order of operations: Modify Carry → Preform Operation → Shift Result → Evaluate Skip Condition

```
NOT <(NOTHING)|SET|CLR|INV>, ACD, ACS, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

## And
Compute the bitwise and of the source and destination. Does not affect the carry bit.

Order of operations: Modify Carry → Preform Operation → Shift Result → Evaluate Skip Condition

```
AND <(NOTHING)|SET|CLR|INV>, ACD, ACS, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

## Or
Compute the bitwise or of the source and destination. Alters the carry bit if `ACS < ACD`

```
OR ACD, ACS

Decodes as:
NOT ACS, ACS
AND ACD, ACS
ADC ACD, ACS ; Compliments carry bit if ACS < ACD
NOT ACS, ACS
```

## Xor
Compute the bitwise xor of the source and destination. Destroys the carry bit.

```
XOR ACD, ACS

Decodes as:
PSH AC3
MOV AC3, ACD
AND CLR, AC3, ACS, SHL ; Destroys carry
ADD ACD, ACS
SUB ACD, AC3
POP AC3
```

The following alternate syntax is a vailable is you do not wish to use the stack. This uses fewer instructions but needs an extra temporary accumulator and may be preferable on systems without a hardware stack.

```
XOR ACD, ACS, ACT#

Decodes as:
MOV ACT#, ACD
AND CLR, ACT#, ACS, SHL ; Destroys carry
ADD ACD, ACS
SUB ACD, ACT#
```

## Increment
Add one to the source. If the value in source was 0xFFFF, invert the carry bit.

Order of operations: Modify Carry → Preform Operation → Shift Result → Evaluate Skip Condition

```
INC <(NOTHING)|SET|CLR|INV>, ACD, ACS, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

## Decrement
Subtract one from the source. If the value in source was 0x0000, invert the carry bit.

Order of operations: Modify Carry → Preform Operation → Shift Result → Evaluate Skip Condition

```
DEC <(NOTHING)|SET|CLR|INV>, ACD, ACS, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>

Decodes as:
NEG <(NOTHING)|SET|CLR|INV>, ACD, ACS
INV ACD, ACD, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

## Add Complement
Invert all the bits in source and add to destination. If the value in source is less than the value in destination, invert the carry bit.

Order of operations: Modify Carry → Preform Operation → Shift Result → Evaluate Skip Condition

```
ADC <(NOTHING)|SET|CLR|INV>, ACD, ACS, <(NOTHING)|SHL|SHR|SWP>, <(NOTHING)|LSKP|LCZ|LCN|LRZ|LRN|LEZ|LBN|TSKP|TCZ|TCN|TRZ|TRN|TEZ|TBN>
```

# Stack Instructions
A stack overflow interrupt is generated every time a `PSH` or `SAV` results in a word being saved to an address which is an integral multiple of `256 (decimal)` or `0xFF (hex)`. The interrupt handler should check to see if the stack pointer exceeds the value present at the `STK_LMT` label and act accordingly.

## Push
Pushes the contents of the specified accumulator to onto the stack and increments the stack pointer by one.
```
PSH ACS
```

## Pop
Pops a word off the stack and places it into the specified accumulator and decrements the stack pointer by one.
```
POP ACD
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
These instructions allow for the control of I/O devices which are addressed by a 6-bit device code.

## Modifiers
I/O Control Signal Modifiers
```
S = Sets the busy flag to 1
C = Clears the busy flag to 0
P = Sends a pulse to the device
```

I/O Skip Conditions
```
BN = Branch if Busy flag is 1
BZ = Branch if Busy flag is 0
DN = Branch if Done flag is 1
DZ = Branch if Doen flag is 0
```

## Flag Instructions
Sends the specified control signal to an I/O device
```
IO[Nothing|S|C|P] <6-Bit Device Code>
```

Checks either the Busy or Done flag based on the specified skip condition and then skips the next insturction if it passes.
```
IO[BN|BZ|DN|DZ] <6-Bit Device Code>
```

## Data Instructions
Transfers the contents of the specified accumulator to/from the A, B, or C register of an I/O device. Additionally sends the specified control signal to the I/O device.

```
IO[A|B|C][Nothing|S|C|P] ACD, <6-Bit Device Code>
IO[A|B|C][Nothing|S|C|P] <6-Bit Device Code>, ACS
```

# Move Register
These instructions are used to transfer between an accumulator and another register. Some additional arguments may be allowed/required depending on the type transfer.

```  
MVR ACD, <Register> : Transfer from Register to ACD
MVR <Register>, ACS : Transfer from ACS to Register
```

## System Control Registers

Transfer AC to the interrupt mask. Do not use while interrupts are enabled. Modifies the interrupt enable flag as desired.
```
MVR IMSK, ACS, <Nothing|INTEN|INTDS>
```

Transfer the 6-bit device code of the interrupting device to AC. Modifies the interrupt enable flag as desired.
```
MVR ACS, IDEV, <Nothing|INTEN|INTDS>
```

Transfer the contents of the front panel's switches to AC. Modifies the interrupt enable flag as desired.
```
MVR ACS, SWR, <Nothing|INTEN|INTDS>
```

## Stack Pointer
```  
MVR ACD, SP
MVR SP, ACS
```

## Frame Pointer
```
MVR ACD, FP
MVR FP, ACS
```

## Floating Point Status
```
MVR ACD, FPST
MVR FPST, ACS
```

# System Instructions
## Reset
Send a reset pulse to all I/O devices. Modifies the interrupt enable flag as desired.
```
IORST, <Nothing|INTEN|INTDS>
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
TRAP AC, <9-Bit Value>
TRAP AC, AC, <7-Bit Value>
```
