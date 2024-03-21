#objdump: -r -j .text
#name: MIPS JALR reloc (o32)
#as: -32
#source: jalr3.s

.*: +file format .*mips.*

RELOCATION RECORDS FOR \[\.text\]:
OFFSET +TYPE +VALUE
00000000 R_MIPS_JALR       \$bar
00000008 R_MIPS_JALR       \$bar
