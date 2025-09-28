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
 0000 5cbc0000 5cbc0800 5cbc1000 5c9c0000  .*
 0010 5c9c0400 5c9c0800 5cfc0000 5cfc0400  .*
 0020 5cfc0800 .*
