  .text
viemu_ptr: viemu
bit_pat: 0xAA55
bit_pat_ptr: >bit_pat
lsb_empty_ptr: >lsb_empty
msb_empty_ptr: <msb_empty
start: .ent start
  LDA 0, viemu_ptr ; Initialize Virtual Instruction Emulation
  STA 0, 047
  LDA 2, bit_pat
  MOVOS 2, 3
  LDA 0, lsb_empty_ptr
  LDA 1, bit_pat_ptr
  LDB 1, 1
  STB 1, 0
  HALT
  .data
lsb_empty: 0xFF00
msb_empty: 0x00FF
  .text
  actbl_ptr: ac0 ; Need a temp register to store PC in
  ac3_ptr: ac3
viemu: .glob viemu ; Entry point, stored in 047
  ; Exchange 046 with AC3
  STA 3, @actbl_ptr ; Save return address
  LDA 3, 046 ; Move AC3 into trap_ac3
  STA 3, @ac3_ptr
  LDA 3, @actbl_ptr ; Move return address into 046
  STA 3, 046
  ; System state now mirrors an actual trap instruction
  LDA 3, actbl_ptr ; AC3 will contain pointer to actbl for rest of routine
  STA 0, 0, 3
  STA 1, 1, 3
  STA 2, 2, 3
  SUBCL 0, 0 ; Store carry
  STA 0, 4, 3
  ; Compute emutbl offset
  LDA 0, @046 ; Load trap instruction
  LDA 2, emutbl_mask ; Load trap mask: 11111110
  MOVR 0, 0 ; Shift right 3 times
  MOVR 0, 0
  MOVR 0, 0
  ANDZR 0, 2 ; AND with mask and shift right to align trap number in AC2
  LDA 1, emutbl_ptr
  ADD 1, 2 ; AC2 now contains pointer into trap table
  ; Get Destination Accumulator (bits 3 & 4 of trap)
  LDA 1, ac_mask
  MOVS 0, 0 ; Bits 14-15 now contain DST AC number
  AND 0, 1 ; AC1 now contains DST AC number
  ADD 3, 1 ; AC1 now contains pointer to DST AC
  STA 1, 8, 3 ; Store pointer to DST AC in dac
  ; Get Source Accumulator (bits 1 & 2 of trap)
  LDA 1, ac_mask
  MOVR 0, 0
  MOVR 0, 0 ; Bits 14-15 now contain SRC AC number
  AND 0, 1 ; AC1 now contians SRC AC number
  ADD 3, 1 ; AC1 now contains pointer to SRC AC
  STA 1, 7, 3 ; Store pointer to SRC AC in sac
  ; Enter trap table
  JMP @0, 2
emutbl_mask: 0xFE
ac_mask: 3
emutbl_ptr: emutbl
viemu_exit: ; Reset CPU state
  LDA 3, actbl_ptr ; Load pointer to ac table
  LDA 0, 4, 3 ; Restore carry
  MOVR 0, 0
  LDA 0, 0, 3 ; Restore registers
  LDA 1, 1, 3
  LDA 2, 2, 3
  LDA 3, 3, 3
  ISZ 046 ; Return to address after TRAP
  JMP @046
  .data
emutbl: trap_syscall ; 0
  ; Multiply & Divide instructions
  trap_mul ; 1
  trap_div ; 2
  ; Nova 3 instructions
  trap_psha ; 3
  trap_popa ; 4
  trap_sav ; 5
  trap_ret ; 6
  trap_mtsp ; 7
  trap_mtfp ; 8
  trap_mfsp ; 9
  trap_mffp ; 10
  ; Nova 4 instructions
  trap_ldb ; 11
  trap_stb ; 12
  trap_muls ; 13
  trap_divs ; 14
  ; F9445 instructions
  trap_or ; 15
  trap_norm ; 16
  trap_slld ; 17
  trap_sald ; 18
  trap_slrd ; 19
  trap_sard ; 20
  trap_popj ; 21
  trap_pshr ; 22
  trap_topr ; 23
  trap_topw ; 24
  trap_dsp ; 25
  ; Virtual instructions
  trap_sign ; 26
  trap_xor ; 27
  .LOC 120 ; Allocate space for rest of table (aprox 128 words)
  .bss
ac0: 1 ; These registers temp store Nova registers
ac1: 1
ac2: 1
ac3: 1
cbit: 1
stack: 1
frame: 1
sac: 1 ; Points to the source accumulator
dac: 1 ; Points to the destination accumulator
  .text
trap_syscall: HALT
  .text
trap_mul: HALT
  .text
trap_div: HALT
  .text
trap_psha: HALT
  .text
trap_popa: HALT
  .text
trap_sav: HALT
  .text
trap_ret: HALT
  .text
trap_mtsp: HALT
  .text
trap_mtfp: HALT
  .text
trap_mfsp: HALT
  .text
trap_mffp: HALT
  .text
ldb_bytemask: 0xFF
ldb_exit: viemu_exit
trap_ldb: LDA 2, @7, 3 ; Load contents of SRC AC (AC3 still holds pointer to actbl)
  MOVZR 2, 2 ; Align address
  LDA 0, 0, 2 ; Load the word containing our byte into AC0
  MOV 2, 2, SNC ; Swap bytes if needed
  MOVS 0, 0
  LDA 1, ldb_bytemask
  AND 1, 0 ; AC0 now contains just our byte
  STA 0, @8, 3 ; Store our byte in DST AC
  JMP @ldb_exit
  .text
stb_bytemask: 0xFF
stb_exit: viemu_exit
trap_stb: LDA 0, @7, 3 ; Load contents of SRC AC into AC0 (AC3 still contains pointer to actbl)
  LDA 1, stb_bytemask
  AND 1, 0 ; AC0 now contains byte to store
  LDA 2, @8, 3 ; Load contents of DST AC into AC2
  MOVZR 2, 2 ; Align byte pointer
  LDA 1, 0, 2 ; Load word to store byte in into AC1
  MOV 2, 2, SNC ; Swap bytes if needed
  MOVS 1, 1
  LDA 2, stb_bytemask ; We loose the contents of DST AC here
  AND 2, 1
  ADD 0, 1 ; AC1 now contains stored byte
  LDA 2, @8, 3 ; Load contents of DST AC back into AC2
  MOVZR 2, 2, SNC ; Align byte pointer and skip switch if needed
  MOVS 1, 1
  STA 1, 0, 2 ; Store word back into memory
  JMP @stb_exit
  .text
trap_muls: HALT
  .text
trap_divs: HALT
  .text
trap_or: HALT
  .text
trap_norm: HALT
  .text
trap_slld: HALT
  .text
trap_sald: HALT
  .text
trap_slrd: HALT
  .text
trap_sard: HALT
  .text
trap_popj: HALT
  .text
trap_pshr: HALT
  .text
trap_topr: HALT
  .text
trap_topw: HALT
  .text
trap_dsp: HALT
  .text
trap_sign: HALT
  .text
trap_xor: HALT
