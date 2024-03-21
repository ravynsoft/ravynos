#as: -mcpu=arc700
#objdump: -dr --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:

[0-9a-f]+ <.text>:
   0:	1100 0000           	ld	r0,\[r1\]
   4:	1601 0005           	ld	r5,\[r6,1\]
   8:	1600 7013 0000 0000 	ld	r19,\[0\]
			c: R_ARC_32_ME	foo
  10:	120a 0204           	ld.aw	r4,\[r2,10\]
  14:	1600 7001 0000 0384 	ld	r1,\[0x384\]
  1c:	130f 0082           	ldb	r2,\[r3,15\]
  20:	14fe 8103           	ld[hw]+	r3,\[r4,-2\]
  24:	212a 0080           	lr	r1,\[r2\]
  28:	216a 0500           	lr	r1,\[0x14\]
  2c:	206a 0000           	lr	r0,\[status\]
