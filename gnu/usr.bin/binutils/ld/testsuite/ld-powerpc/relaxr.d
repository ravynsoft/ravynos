
.*:     file format .*

Disassembly of section .text:

00000000 <_start>:
   0:	(48 00 00 01|01 00 00 48) 	bl      0 <_start>
			0: R_PPC_REL24	near
   4:	(48 00 00 01|01 00 00 48) 	bl      4 <_start\+0x4>
			4: R_PPC_REL24	far
   8:	(48 00 00 01|01 00 00 48) 	bl      8 <_start\+0x8>
			8: R_PPC_REL24	near
   c:	(48 00 00 01|01 00 00 48) 	bl      c <_start\+0xc>
			c: R_PPC_REL24	far
  10:	(48 00 00 00|00 00 00 48) 	b       10 <_start\+0x10>
			10: R_PPC_REL24	_start
