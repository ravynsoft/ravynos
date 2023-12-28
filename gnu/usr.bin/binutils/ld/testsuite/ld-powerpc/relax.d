
.*:     file format .*

Disassembly of section .text:

00000000 <_start>:
   0:	(48 00 43 21|21 43 00 48) 	bl      4320 <near>
   4:	(48 00 00 11|11 00 00 48) 	bl      14 <_start\+0x14>
   8:	(48 00 43 19|19 43 00 48) 	bl      4320 <near>
   c:	(48 00 00 09|09 00 00 48) 	bl      14 <_start\+0x14>
  10:	(4b ff ff f0|f0 ff ff 4b) 	b       0 <.*>
  14:	(3d 80 80 00|00 80 80 3d) 	lis     r12,-32768
  18:	(39 8c 12 34|34 12 8c 39) 	addi    r12,r12,4660
  1c:	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
  20:	(4e 80 04 20|20 04 80 4e) 	bctr
