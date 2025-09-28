#objdump: -r -j .text
#name MIPS .ehword
#source ehword.s

.*: +file format .*mips.*

RELOCATION RECORDS FOR \[\.text\]:
OFFSET +TYPE +VALUE
00000000 R_MIPS_EH         _ZTI5myExc
