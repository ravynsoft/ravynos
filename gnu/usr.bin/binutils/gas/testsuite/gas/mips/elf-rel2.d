#objdump: -sr -j .text
#name: MIPS ELF reloc 2
#as: -mabi=o64

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
 0000 d7820000 d7820008 d7820010 c7820000  .*
 0010 c7820004 c7820008 8f820000 8f820004  .*
 0020 8f820008 .*
