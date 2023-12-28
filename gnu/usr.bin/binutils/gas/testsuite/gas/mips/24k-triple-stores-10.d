#objdump: -dr -z
#as: -mfix-24k -32
#name: 24K: Triple Store (Intervening data #2)

.*: +file format .*mips.*

Disassembly of section .text:
0+ <.*>:
   0:	a1020000 	sb	v0,0\(t0\)
   4:	a1030008 	sb	v1,8\(t0\)
   8:	00000000 	nop
   c:	a1040010 	sb	a0,16\(t0\)
  10:	00000000 	nop
#pass
