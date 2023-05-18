// Novix COFF

/* COFF String Table Pointer
  * If the string is exactly 8 chars, then no padding is provided otherwise
    a null terminator should be present
    
  * If the first character of the string is null, then this is a pointer into
    the string table
    
  * SYMLEN define should be at least 8, but programs are expected to be able to
    handle a SYMLEN of any value above that
*/

#define SYMLEN 8

union coff_string {
    uint8_t str[SYMLEN]; // Embedded string
    struct {
        uint8_t zero;     // Must be zero if using the ptr
        uint32_t ptr;     // String table pointer
    };
};


/* COFF File Header
  * First thing in a COFF file
    
  * This header must be standard to other COFF formats so that other
    utilities can at least recognize the foreign magic number

  * Magic number should be 0x4447 for Novix
*/

#define F_DGMAGIC 0x4447 // ASCII characters "DG"

// File header flag constants
#define F_NORELOC   0x0001 // No relocation data is present
#define F_EXEC      0x0002 // This is an executable file
#define F_NOLINE    0x0004 // No line number information is present
#define F_NOLOCAL   0x0008 // No local symbols are present

struct coff_fhdr {
    uint16_t f_magic;   // Magic number
    uint16_t f_secnum;  // Number of sections
    uint32_t f_time;    // 32-bit UNIX timestamp
    uint32_t f_symptr;  // File offset for symbol table
    uint32_t f_symnum;  // Number of symbol table entries
    uint16_t f_exehdr;  // Size of the executable header
    uint16_t f_flags;   // See above for flag definitions
};

/* COFF Executable header
    * Comes in 3 formats for Nova, Eclipse, and MV/Eclipse architectures
    
    * While these headers are technically COFF compliant, they're only
      supported by Novix
    
    * Executable flags denote the presence of instructions which require
      specific features not present on all devices. These should be
      substituted out with compatibility routines
*/

#define E_NVMAGIC 0x4E56 // ASCII characters "NV" (Nova)
#define E_ECMAGIC 0x4543 // ASCII characters "EC" (Eclipse)
#define E_MVMAGIC 0x4D56 // ASCII characters "MV" (MV/Eclipse)

// Executable flag constants for Nova
#define E_CMPAT 0x0001  // Program supports instruction compatibility routines
#define E_UMDV  0x0002  // Unsigned multiply and divide
#define E_SMDV  0x0004  // Signed multiplu and divide
#define E_TRAP  0x0008  // Trap instruction
#define E_STACK 0x0010  // Stack instructions
#define E_BYTE  0x0012  // Byte instructions
#define E_FLOAT 0x0014  // Floating point instructions

struct coff_nv_ehdr {
    uint16_t e_magic;   // Magic number, 0x4E56 for this struct
    uint16_t e_ver;     // Version number
    uint16_t e_entry;   // Entry pointer
    uint16_t e_flags;   // Target system feature flags
};

struct coff_ec_ehdr {
    uint16_t e_magic;   // Magic number, 0x4E56 for this struct
    uint16_t e_ver;     // Version number
};

struct coff_mv_ehdr {
    uint16_t e_magic;   // Magic number, 0x4E56 for this struct
    uint16_t e_ver;     // Version number
};

/* COFF Section Header

*/

struct coff_sect {
    union coff_string s_name;   // Section name
    uint32_t s_paddr;           // Physical address
    uint32_t s_vaddr;           // Virtual address
    uint32_t s_size;            // Size in bytes of section data
    uint32_t s_secptr;          // File pointer to section data
    uint32_t s_relptr;          // File pointer to relocation entries
    uint32_t s_linptr;          // File pointer to line entries
    uint16_t s_relnum;          // Number of relocation entries
    uint16_t s_linnum;          // Number of line entries
    uint16_t s_flags;           // Flags
};
