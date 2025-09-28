#objdump: -rsj .data
#name: .equ redefinitions (3)
#identical to ../all/redef, we just need to reverse the relocs.
#source: ../all/redef3.s

.*: .*

RELOCATION RECORDS FOR .*
.*
0+10.*sym
0+08.*xtrn
0+00.*(here|\.data)
#...
Contents of section \.data:
 0000 00000000 11111111 00000000 22222222[ 	]+................[ 	]*
 0010 00000000 .*
#pass
