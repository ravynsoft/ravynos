#as: -mcpu=arc700
#objdump: -dr --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:

[0-9a-f]+ <.text>:
   0:	2130 0080           	ld	r0,\[r1,r2\]
   4:	2132 0080           	ldb	r0,\[r1,r2\]
   8:	2370 0101           	ld.aw	r1,\[r3,r4\]
   c:	2235 00c1           	ld[hw]+.x	r1,\[r2,r3\]
  10:	2375 0102           	ld[hw]+.aw.x	r2,\[r3,r4\]
  14:	1600 7000 0000 0000 	ld	r0,\[0\]
  1c:	111e 0000           	ld	r0,\[r1,30\]
  20:	12ec 8001           	ld	r1,\[r2,-20\]
