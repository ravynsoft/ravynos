#objdump: -drz
#as: -mfix-24k -32
#name: 24K: Triple store (Intervening data #1)

.*: +file format .*mips.*

Disassembly of section .text:

0+ <.*>:
   0:	a1020000 	sb	v0,0\(t0\)
   4:	00000000 	nop
   8:	a1030008 	sb	v1,8\(t0\)
   c:	00000000 	nop
  10:	a1040010 	sb	a0,16\(t0\)
  14:	00000000 	nop
#pass
