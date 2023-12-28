#as: -march=mips2 -mabi=32 -KPIC
#readelf: --relocs
#name: MIPS ELF reloc 13 (MIPS16 version)

Relocation section '\.rel\.text' at offset .* contains 9 entries:
 *Offset * Info * Type * Sym\.Value * Sym\. Name
0+0002 * 0+..66 * R_MIPS16_GOT16 * 0+0000 * \.data
0+0016 * 0+..69 * R_MIPS16_LO16 * 0+0000 * \.data
0+0012 * 0+..66 * R_MIPS16_GOT16 * 0+0000 * \.data
0+001a * 0+..69 * R_MIPS16_LO16 * 0+0000 * \.data
# The next two lines could be in either order.
0+000e * 0+..66 * R_MIPS16_GOT16 * 0+0000 * \.rodata
0+000a * 0+..66 * R_MIPS16_GOT16 * 0+0000 * \.rodata
0+001e * 0+..69 * R_MIPS16_LO16 * 0+0000 * \.rodata
0+0006 * 0+..66 * R_MIPS16_GOT16 * 0+0000 * \.bss
0+0022 * 0+..69 * R_MIPS16_LO16 * 0+0000 * \.bss
#pass
