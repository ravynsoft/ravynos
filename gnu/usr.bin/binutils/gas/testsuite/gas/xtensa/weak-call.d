#as: 
#objdump: -r -j .text
#name: relaxing calls to weak symbols

.*: +file format .*xtensa.*

RELOCATION RECORDS FOR \[\.text\]:
OFFSET +TYPE +VALUE
00000000 R_XTENSA_SLOT0_OP  weakdef
00000003 R_XTENSA_SLOT0_OP  \.literal
00000003 R_XTENSA_ASM_EXPAND  weakref
