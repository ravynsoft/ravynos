#as: 
#objdump: -r -j .literal -j .text
#name: pc-relative relocs

.*: +file format .*xtensa.*

RELOCATION RECORDS FOR \[\.literal\]:
OFFSET +TYPE +VALUE
00000000 R_XTENSA_32_PCREL  foo


RELOCATION RECORDS FOR \[\.text\]:
OFFSET +TYPE +VALUE
00000003 R_XTENSA_SLOT0_OP  \.literal
00000006 R_XTENSA_32_PCREL  foo
0000000a R_XTENSA_32_PCREL  \.text\+0x00000003
0000000e R_XTENSA_32_PCREL  \.text\+0x00000006


