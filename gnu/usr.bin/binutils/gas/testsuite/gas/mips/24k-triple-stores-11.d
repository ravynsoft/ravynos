#objdump: -dz
#as: -mfix-24k -32
#name: 24K: Triple Store (gprel relocs)

.*: +file format .*mips.*

Disassembly of section .text:
0+ <.*>:
   0:	00842020 	add	a0,a0,a0
   4:	00842020 	add	a0,a0,a0
   8:	00842020 	add	a0,a0,a0
   c:	00842020 	add	a0,a0,a0
  10:	af820000 	sw	v0,0\(gp\)
  14:	af830000 	sw	v1,0\(gp\)
  18:	00000000 	nop
  1c:	af840000 	sw	a0,0\(gp\)
  20:	00000000 	nop
#pass
