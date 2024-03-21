# name: bsr2 - csky
#as: -mcpu=ck610
#objdump: -r

.*: +file format .*csky.*

RELOCATION RECORDS FOR \[\.text\]:
#...
[0-9a-f]*\s*R_CKCORE_PCREL_IMM11BY2\s*hello
