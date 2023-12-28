#objdump: -dr
#as: -mfix-24k -32
#source: 24k-triple-stores-2-llsc.s
#name: 24K: Triple Store (Range Check, sc)

.*: +file format .*mips.*

Disassembly of section .text:
0+ <.*>:
   0:	7fa21026 	sc	v0,32\(sp\)
   4:	7fa30426 	sc	v1,8\(sp\)
   8:	00000000 	nop
   c:	7fa4fc26 	sc	a0,-8\(sp\)
  10:	7fa50026 	sc	a1,0\(sp\)
  14:	00000000 	nop
  18:	7fa61026 	sc	a2,32\(sp\)
	\.\.\.
