#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	8b1e0210 	add	x16, x16, x30
   4:	f90003b0 	str	x16, \[x29\]
   8:	f94003b1 	ldr	x17, \[x29\]
   c:	f90003b0 	str	x16, \[x29\]
  10:	f94003b1 	ldr	x17, \[x29\]
  14:	f900001f 	str	xzr, \[x0\]
