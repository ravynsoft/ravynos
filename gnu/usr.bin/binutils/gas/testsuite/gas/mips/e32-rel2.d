#objdump: -sr -j .text
#name: MIPS ELF reloc 2 (32-bit)
#as: -mabi=32 -march=mips1
#source: elf-rel2.s

# Test the GPREL and LITERAL generation.
# FIXME: really this should check that the contents of .sdata, .lit4,
# and .lit8 are correct too.

.*:     file format .*

RELOCATION RECORDS FOR \[\.text\]:
OFFSET +TYPE +VALUE
0+0000000 R_MIPS_LITERAL    \.lit8
0+0000004 R_MIPS_LITERAL    \.lit8
0+0000008 R_MIPS_LITERAL    \.lit8
0+000000c R_MIPS_LITERAL    \.lit8
0+0000010 R_MIPS_LITERAL    \.lit8
0+0000014 R_MIPS_LITERAL    \.lit8
0+0000018 R_MIPS_LITERAL    \.lit4
0+000001c R_MIPS_LITERAL    \.lit4
0+0000020 R_MIPS_LITERAL    \.lit4
0+0000024 R_MIPS_GPREL16    \.sdata
0+0000028 R_MIPS_GPREL16    \.sdata
0+000002c R_MIPS_GPREL16    \.sdata


Contents of section \.text:
 0000 c7830000 c7820004 c7830008 c782000c  .*
 0010 c7830010 c7820014 c7820000 c7820004  .*
 0020 c7820008 8f820000 8f820004 8f820008  .*
