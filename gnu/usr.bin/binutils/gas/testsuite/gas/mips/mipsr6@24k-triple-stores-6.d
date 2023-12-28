#objdump: -dr
#as: -mfix-24k -32 -EB
#name: 24K: Triple Store (Store Macro Check)
#source: 24k-triple-stores-6.s

.*: +file format .*mips.*

Disassembly of section .text:
0+ <.*>:
   0:	e7a00050 	swc1	\$f0,80\(sp\)
   4:	e7a20058 	swc1	\$f2,88\(sp\)
   8:	00000000 	nop
   c:	e7a40060 	swc1	\$f4,96\(sp\)
  10:	0000000d 	break
  14:	f7a00050 	sdc1	\$f0,80\(sp\)
  18:	f7a20058 	sdc1	\$f2,88\(sp\)
  1c:	00000000 	nop
  20:	f7a40060 	sdc1	\$f4,96\(sp\)
  24:	0000000d 	break
	\.\.\.
