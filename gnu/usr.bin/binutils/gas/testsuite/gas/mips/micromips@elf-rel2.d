#objdump: -sr -j .text
#name: MIPS ELF reloc 2
#source: elf-rel2.s
#as: -mabi=o64

# Test the GPREL and LITERAL generation (microMIPS).
# FIXME: really this should check that the contents of .sdata, .lit4,
# and .lit8 are correct too.

.*: +file format .*mips.*

RELOCATION RECORDS FOR \[\.text\]:
OFFSET +TYPE +VALUE
0+0000000 R_MICROMIPS_LITERAL  \.lit8
0+0000004 R_MICROMIPS_LITERAL  \.lit8
0+0000008 R_MICROMIPS_LITERAL  \.lit8
0+000000c R_MICROMIPS_LITERAL  \.lit4
0+0000010 R_MICROMIPS_LITERAL  \.lit4
0+0000014 R_MICROMIPS_LITERAL  \.lit4
0+0000018 R_MICROMIPS_GPREL16  \.sdata
0+000001c R_MICROMIPS_GPREL16  \.sdata
0+0000020 R_MICROMIPS_GPREL16  \.sdata


Contents of section \.text:
 0000 bc5c0000 bc5c0008 bc5c0010 9c5c0000  .*
 0010 9c5c0004 9c5c0008 fc5c0000 fc5c0004  .*
 0020 fc5c0008 .*
