#name: x86-64 rela relocs w/ non-zero relocated fields
#objdump: -rsj .data

.*: +file format .*

RELOCATION RECORDS FOR \[\.data\]:

OFFSET +TYPE +VALUE
0*0 R_X86_64_64 *q
0*8 R_X86_64_32 *l

Contents of section .data:
 0+0 11 ?11 ?11 ?11 22 ?22 ?22 ?22 33 ?33 ?33 ?33 44 ?44 ?44 ?44 .*
