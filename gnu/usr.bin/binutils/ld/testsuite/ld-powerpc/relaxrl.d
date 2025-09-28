
.*:     file format .*

Disassembly of section .text:

0+ <_start>:
 +0:	(48 00 00 15|15 00 00 48) 	bl      14 <_start\+0x14>
 +4:	(48 00 00 21|21 00 00 48) 	bl      24 <_start\+0x24>
 +8:	(48 00 00 0d|0d 00 00 48) 	bl      14 <_start\+0x14>
			8: R_PPC_NONE	\*ABS\*
 +c:	(48 00 00 19|19 00 00 48) 	bl      24 <_start\+0x24>
			c: R_PPC_NONE	\*ABS\*
 +10:	(48 00 00 00|00 00 00 48) 	b       10 <_start\+0x10>
			10: R_PPC_REL24	_start
 +14:	(3d 80 00 00|00 00 80 3d) 	lis     r12,0
			1(6|4): R_PPC_ADDR16_HA	near
 +18:	(39 8c 00 00|00 00 8c 39) 	addi    r12,r12,0
			1(a|8): R_PPC_ADDR16_LO	near
 +1c:	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
 +20:	(4e 80 04 20|20 04 80 4e) 	bctr
 +24:	(3d 80 00 00|00 00 80 3d) 	lis     r12,0
			2(6|4): R_PPC_ADDR16_HA	far
 +28:	(39 8c 00 00|00 00 8c 39) 	addi    r12,r12,0
			2(a|8): R_PPC_ADDR16_LO	far
 +2c:	(7d 89 03 a6|a6 03 89 7d) 	mtctr   r12
 +30:	(4e 80 04 20|20 04 80 4e) 	bctr
	\.\.\.
