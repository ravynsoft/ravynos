#objdump: -Dr -Mppc
#as:  --generate-missing-build-notes=no
#name: PowerPC test 2

.*

Disassembly of section \.text:

0+0000000 <foo>:
   0:	(60 00 00 00|00 00 00 60) 	nop
   4:	(60 00 00 00|00 00 00 60) 	nop
   8:	(60 00 00 00|00 00 00 60) 	nop
   c:	(48 00 00 04|04 00 00 48) 	b       10 <foo\+0x10>
  10:	(48 00 00 08|08 00 00 48) 	b       18 <foo\+0x18>
  14:	(48 00 00 00|00 00 00 48) 	b       .*
			14: R_PPC_REL24	x
  18:	(48 00 00 0.|0. 00 00 48) 	b       .*
			18: R_PPC_REL24	\.data\+0x4
  1c:	(48 00 00 00|00 00 00 48) 	b       .*
			1c: R_PPC_REL24	z
  20:	(48 00 00 ..|.. 00 00 48) 	b       .*
			20: R_PPC_REL24	z\+0x14
  24:	(48 00 00 04|04 00 00 48) 	b       28 <foo\+0x28>
  28:	(48 00 00 00|00 00 00 48) 	b       .*
			28: R_PPC_REL24	a
  2c:	(48 00 00 50|50 00 00 48) 	b       7c <apfour>
  30:	(48 00 00 0.|0. 00 00 48) 	b       .*
			30: R_PPC_REL24	a\+0x4
  34:	(48 00 00 4c|4c 00 00 48) 	b       80 <apfour\+0x4>
  38:	(48 00 00 00|00 00 00 48) 	b       .*
			38: R_PPC_LOCAL24PC	a
  3c:	(48 00 00 40|40 00 00 48) 	b       7c <apfour>
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
  70:	(00 00 00 08|08 00 00 00) 	\.long 0x8
  74:	(00 00 00 08|08 00 00 00) 	\.long 0x8

0+0000078 <a>:
  78:	(00 00 00 00|00 00 00 00) 	\.long 0x0
			78: R_PPC_ADDR32	a

0+000007c <apfour>:
	\.\.\.
			7c: R_PPC_ADDR32	\.text\+0x7c
			80: R_PPC_ADDR32	\.text\+0x7c
  84:	(ff ff ff fc|fc ff ff ff) 	fnmsub  f31,f31,f31,f31
	\.\.\.
			88: R_PPC_ADDR32	\.text\+0x7e
  90:	(60 00 00 00|00 00 00 60) 	nop
  94:	(40 a5 ff fc|fc ff a5 40) 	ble-    cr1,90 <apfour\+0x14>
  98:	(41 a9 ff f8|f8 ff a9 41) 	bgt-    cr2,90 <apfour\+0x14>
  9c:	(40 8d ff f4|f4 ff 8d 40) 	ble\+    cr3,90 <apfour\+0x14>
  a0:	(41 91 ff f0|f0 ff 91 41) 	bgt\+    cr4,90 <apfour\+0x14>
  a4:	(40 95 00 10|10 00 95 40) 	ble-    cr5,b4 <nop>
  a8:	(41 99 00 0c|0c 00 99 41) 	bgt-    cr6,b4 <nop>
  ac:	(40 bd 00 08|08 00 bd 40) 	ble\+    cr7,b4 <nop>
  b0:	(41 a1 00 04|04 00 a1 41) 	bgt\+    b4 <nop>
Disassembly of section \.data:

0+0000000 <x>:
   0:	00 00 00 00 	\.long 0x0

0+0000004 <y>:
   4:	00 00 00 00 	\.long 0x0
