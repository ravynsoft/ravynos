#objdump: -sr -j .text
#name: MIPS ELF reloc 2 (32-bit)
#source: elf-rel2.s
#as: -32 -march=mips3

# Test the GPREL and LITERAL generation.
# FIXME: really this should check that the contents of .sdata, .lit4,
# and .lit8 are correct too.

.*:     file format .*

RELOCATION RECORDS FOR \[\.text\]:
OFFSET +TYPE +VALUE
0+0000000 R_MIPS_LITERAL    \.lit8
0+0000004 R_MIPS_LITERAL    \.lit8
0+0000008 R_MIPS_LITERAL    \.lit8
0+000000c R_MIPS_LITERAL    \.lit4
0+0000010 R_MIPS_LITERAL    \.lit4
0+0000014 R_MIPS_LITERAL    \.lit4
0+0000018 R_MIPS_GPREL16    \.sdata
0+000001c R_MIPS_GPREL16    \.sdata
0+0000020 R_MIPS_GPREL16    \.sdata


Contents of section \.text:
 0000 000082d7 080082d7 100082d7 000082c7  .*
 0010 040082c7 080082c7 0000828f 0400828f  .*
 0020 0800828f .*
