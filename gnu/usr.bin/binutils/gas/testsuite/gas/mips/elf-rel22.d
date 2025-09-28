#as: -march=mips3 -mabi=64
#readelf: --relocs
#name: MIPS ELF reloc 22

Relocation section '\.rela\.text' .*:
.*
.* R_MIPS_LO16 * 4
 * Type2: R_MIPS_SUB *
 * Type3: R_MIPS_LO16 *
.* R_MIPS_LO16 * 4
 * Type2: R_MIPS_SUB *
 * Type3: R_MIPS_LO16 *
.* R_MIPS_GPREL16 * 123456
 * Type2: R_MIPS_HI16 *
 * Type3: R_MIPS_NONE *
.* R_MIPS_GPREL16 * 234567
 * Type2: R_MIPS_SUB *
 * Type3: R_MIPS_LO16 *
