#objdump: -Dr
#as:  --generate-missing-build-notes=no
#name: PowerPC 64-bit test 2

.*

Disassembly of section \.text:

0000000000000000 <foo>:
   0:	(60 00 00 00|00 00 00 60) 	nop
   4:	(60 00 00 00|00 00 00 60) 	nop
   8:	(60 00 00 00|00 00 00 60) 	nop
   c:	(48 00 00 04|04 00 00 48) 	b       10 <foo\+0x10>
  10:	(48 00 00 08|08 00 00 48) 	b       18 <foo\+0x18>
  14:	(48 00 00 00|00 00 00 48) 	b       .*
			14: R_PPC64_REL24	x
  18:	(48 00 00 0.|0. 00 00 48) 	b       .*
			18: R_PPC64_REL24	\.data\+0x4
  1c:	(48 00 00 00|00 00 00 48) 	b       .*
			1c: R_PPC64_REL24	z
  20:	(48 00 00 ..|.. 00 00 48) 	b       .*
			20: R_PPC64_REL24	z\+0x14
  24:	(48 00 00 04|04 00 00 48) 	b       28 <foo\+0x28>
  28:	(48 00 00 00|00 00 00 48) 	b       .*
			28: R_PPC64_REL24	a
  2c:	(48 00 00 48|48 00 00 48) 	b       74 <apfour>
  30:	(48 00 00 0.|0. 00 00 48) 	b       .*
			30: R_PPC64_REL24	a\+0x4
  34:	(48 00 00 44|44 00 00 48) 	b       78 <apfour\+0x4>
	\.\.\.
			38: R_PPC64_ADDR32	\.text\+0x38
			3c: R_PPC64_ADDR32	\.text\+0x44
			40: R_PPC64_REL32	x
			44: R_PPC64_REL32	x\+0x4
			48: R_PPC64_REL32	z
			4c: R_PPC64_REL32	\.data\+0x4
			50: R_PPC64_ADDR32	x
			54: R_PPC64_ADDR32	\.data\+0x4
			58: R_PPC64_ADDR32	z
			5c: R_PPC64_ADDR32	x-0x4
			60: R_PPC64_ADDR32	\.data
			64: R_PPC64_ADDR32	z-0x4
  68:	(00 00 00 08|08 00 00 00) 	\.long 0x8
  6c:	(00 00 00 08|08 00 00 00) 	\.long 0x8

0000000000000070 <a>:
  70:	(00 00 00 00|00 00 00 00) 	\.long 0x0
			70: R_PPC64_ADDR32	a

0000000000000074 <apfour>:
	\.\.\.
			74: R_PPC64_ADDR32	\.text\+0x74
			78: R_PPC64_ADDR32	\.text\+0x74
  7c:	(ff ff ff fc|fc ff ff ff) 	fnmsub  f31,f31,f31,f31
	\.\.\.
			80: R_PPC64_ADDR32	\.text\+0x76
Disassembly of section \.data:

0000000000000000 <x>:
   0:	00 00 00 00 	\.long 0x0

0000000000000004 <y>:
   4:	00 00 00 00 	\.long 0x0
