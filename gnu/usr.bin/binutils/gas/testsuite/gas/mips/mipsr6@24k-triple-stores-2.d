#objdump: -dr
#as: -mfix-24k -32
#source: 24k-triple-stores-2.s
#name: 24K: Triple Store (Range Check)

.*: +file format .*mips.*

Disassembly of section .text:
0+ <.*>:
   0:	a3a20000 	sb	v0,0\(sp\)
   4:	a3a3000a 	sb	v1,10\(sp\)
   8:	00000000 	nop
   c:	a3a4001f 	sb	a0,31\(sp\)
  10:	0000000d 	break
  14:	a7a20000 	sh	v0,0\(sp\)
  18:	a7a3fff0 	sh	v1,-16\(sp\)
  1c:	a7a4ffe0 	sh	a0,-32\(sp\)
  20:	0000000d 	break
  24:	afa20000 	sw	v0,0\(sp\)
  28:	afa3fff8 	sw	v1,-8\(sp\)
  2c:	00000000 	nop
  30:	afa40008 	sw	a0,8\(sp\)
  34:	0000000d 	break
	\.\.\.
