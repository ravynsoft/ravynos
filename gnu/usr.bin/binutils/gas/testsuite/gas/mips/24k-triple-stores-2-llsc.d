#objdump: -dr
#as: -mfix-24k -32
#name: 24K: Triple Store (Range Check, sc)

.*: +file format .*mips.*

Disassembly of section .text:
0+ <.*>:
   0:	e3a20020 	sc	v0,32\(sp\)
   4:	e3a30008 	sc	v1,8\(sp\)
   8:	e3a4fff8 	sc	a0,-8\(sp\)
   c:	00000000 	nop
  10:	e3a50000 	sc	a1,0\(sp\)
  14:	e3a60020 	sc	a2,32\(sp\)
	\.\.\.
