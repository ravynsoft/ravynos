#objdump: -Dr
#name: PowerPC test 1

.*

Disassembly of section \.text:

0+0000000 <foo>:
   0:	(60 00 00 00|00 00 00 60) 	nop
   4:	(60 00 00 00|00 00 00 60) 	nop
   8:	(60 00 00 00|00 00 00 60) 	nop

0+000000c <a>:
   c:	(48 00 00 04|04 00 00 48) 	b       10 <apfour>

0+0000010 <apfour>:
  10:	(48 00 00 08|08 00 00 48) 	b       18 <apfour\+0x8>
  14:	(48 00 00 00|00 00 00 48) 	b       .*
			14: R_PPC_REL24	x
  18:	(48 00 00 0.|0. 00 00 48) 	b       .*
			18: R_PPC_REL24	\.data\+0x4
  1c:	(48 00 00 00|00 00 00 48) 	b       .*
			1c: R_PPC_REL24	z
  20:	(48 00 00 ..|.. 00 00 48) 	b       .*
			20: R_PPC_REL24	z\+0x14
  24:	(48 00 00 04|04 00 00 48) 	b       28 <apfour\+0x18>
  28:	(48 00 00 00|00 00 00 48) 	b       .*
			28: R_PPC_REL24	a
  2c:	(4b ff ff e4|e4 ff ff 4b) 	b       10 <apfour>
  30:	(48 00 00 0.|0. 00 00 48) 	b       .*
			30: R_PPC_REL24	a\+0x4
  34:	(4b ff ff e0|e0 ff ff 4b) 	b       14 <apfour\+0x4>
  38:	(48 00 00 00|00 00 00 48) 	b       .*
			38: R_PPC_LOCAL24PC	a
  3c:	(4b ff ff d4|d4 ff ff 4b) 	b       10 <apfour>
	\.\.\.
			40: R_PPC_ADDR32	\.text\+0x40
			44: R_PPC_ADDR32	\.text\+0x4c
			48: R_PPC_REL32	x
			4c: R_PPC_REL32	x\+0x4
			50: R_PPC_REL32	z
			54: R_PPC_REL32	\.data\+0x4
			58: R_PPC_ADDR32	x
			5c: R_PPC_ADDR32	\.data\+0x4
			60: R_PPC_ADDR32	z
			64: R_PPC_ADDR32	x-0x4
			68: R_PPC_ADDR32	\.data
			6c: R_PPC_ADDR32	z-0x4
  70:	(ff ff ff 9c|9c ff ff ff) 	\.long 0xffffff9c
  74:	(ff ff ff 9c|9c ff ff ff) 	\.long 0xffffff9c
	\.\.\.
			78: R_PPC_ADDR32	a
			7c: R_PPC_ADDR32	\.text\+0x10
			80: R_PPC_ADDR32	\.text\+0x10
  84:	(ff ff ff fc|fc ff ff ff) 	fnmsub  f31,f31,f31,f31
	\.\.\.
			88: R_PPC_ADDR32	\.text\+0x12
Disassembly of section \.data:

0+0000000 <x>:
   0:	00 00 00 00 	\.long 0x0

0+0000004 <y>:
   4:	00 00 00 00 	\.long 0x0
