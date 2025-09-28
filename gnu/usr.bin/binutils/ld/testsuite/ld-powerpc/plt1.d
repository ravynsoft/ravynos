#source: plt1.s
#as: -a32
#objdump: -dr
#target: powerpc*-*-*

.*

Disassembly of section .text:

0+ <_start>:
   0:	(42 9f 00 05|05 00 9f 42) 	bcl     20,4\*cr7\+so,4 .*
   4:	(7f c8 02 a6|a6 02 c8 7f) 	mflr    r30
   8:	(3f de 00 00|00 00 de 3f) 	addis   r30,r30,0
			(a|8): R_PPC_REL16_HA	_GLOBAL_OFFSET_TABLE_\+0x(6|4)
   c:	(3b de 00 0.|0. 00 de 3b) 	addi    r30,r30,.*
			(e|c): R_PPC_REL16_LO	_GLOBAL_OFFSET_TABLE_\+0x(a|8)
  10:	(48 00 00 01|01 00 00 48) 	bl      10 .*
			10: R_PPC_PLTREL24	_exit
  14:	(48 00 00 00|00 00 00 48) 	b       14 .*
			14: R_PPC_REL24	_start
