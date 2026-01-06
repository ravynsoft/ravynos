/*
 * These #defines are needed in arm.c and write_object.c to create the
 * the mach-o arm relocation r_types ARM_RELOC_HALF and ARM_RELOC_HALF_SECTDIFF.
 * The values are internal but can't over lap any of the 4 bit r_type values
 * they they are outside that 4 bit range.  But they are picked so we can use
 * the low two bits for the r_length encoding.
 */

/* internal assembler value for BFD_RELOC_ARM_MOVW */
#define ARM_RELOC_LO16 0x70
/* r_length - low bit is 0 for :lower16:, high bit is 0 for arm instruction */

/* internal assembler value for BFD_RELOC_ARM_MOVT */
#define ARM_RELOC_HI16 0x71
/* r_length - low bit is 1 for :upper16:, high bit is 0 for arm instruction */

/* internal assembler value for BFD_RELOC_ARM_THUMB_MOVW */
#define ARM_THUMB_RELOC_LO16 0x72
/* r_length - low bit is 0 for :lower16:, high bit is 1 for thumb instruction */

/* internal assembler value for BFD_RELOC_ARM_THUMB_MOVT */
#define ARM_THUMB_RELOC_HI16  0x73
/* r_length - low bit is 1 for :upper16:, high bit is 1 for thumb instruction */
