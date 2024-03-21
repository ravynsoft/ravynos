#objdump: -d -Me200z4
#as: -a32 -mbig -me200z4

.*

Disassembly of section \.text:

0+ <\.text>:
   0:	70 00 00 00 	e_li    r0,0
   4:	7c 01 14 04 	lbdcbx  r0,r1,r2
   8:	7c 01 14 44 	lhdcbx  r0,r1,r2
   c:	7c 01 14 84 	lwdcbx  r0,r1,r2
