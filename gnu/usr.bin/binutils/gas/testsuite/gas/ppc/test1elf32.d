#objdump: -Drx
#name: PowerPC Test 1, 32 bit elf

.*
.*
architecture: powerpc:common, flags 0x00000011:
HAS_RELOC, HAS_SYMS
start address 0x00000000

Sections:
Idx Name +Size +VMA +LMA +File off +Algn
  0 \.text +00000050  0+0000  0+0000  .*
 +CONTENTS, ALLOC, LOAD, RELOC, READONLY, CODE
  1 \.data +00000018  0+0000  0+0000  .*
 +CONTENTS, ALLOC, LOAD, RELOC, DATA
  2 \.bss  +00000000  0+0000  0+0000  .*
 +ALLOC
SYMBOL TABLE:
0+0000 l    d  \.text	0+0000 (|\.text)
0+0000 l    d  \.data	0+0000 (|\.data)
0+0000 l    d  \.bss	0+0000 (|\.bss)
0+0000 l       \.data	0+0000 dsym0
0+0004 l       \.data	0+0000 dsym1
0+0004 l       \.data	0+0000 usym0
0+0008 l       \.data	0+0000 usym1
0+0008 l       \.data	0+0000 datpt
0+000c l       \.data	0+0000 dat0
0+0010 l       \.data	0+0000 dat1
0+0014 l       \.data	0+0000 dat2
0+0000         \*UND\*	0+0000 esym0
0+0000         \*UND\*	0+0000 esym1
0+0000         \*UND\*	0+0000 jk


Disassembly of section \.text:

0+0000 <\.text>:
   0:	(80 63 00 00|00 00 63 80) 	lwz     r3,0\(r3\)
			(2|0): R_PPC_ADDR16_LO	\.data
   4:	(80 63 00 0.|0. 00 63 80) 	lwz     r3,.\(r3\)
			(6|4): R_PPC_ADDR16_LO	\.data\+0x4
   8:	(80 63 00 0.|0. 00 63 80) 	lwz     r3,.\(r3\)
			(a|8): R_PPC_ADDR16_LO	\.data\+0x4
   c:	(80 63 00 0.|0. 00 63 80) 	lwz     r3,.\(r3\)
			(e|c): R_PPC_ADDR16_LO	\.data\+0x8
  10:	(80 63 00 00|00 00 63 80) 	lwz     r3,0\(r3\)
			(12|10): R_PPC_ADDR16_LO	esym0
  14:	(80 63 00 00|00 00 63 80) 	lwz     r3,0\(r3\)
			(16|14): R_PPC_ADDR16_LO	esym1
  18:	(38 60 00 04|04 00 60 38) 	li      r3,4
  1c:	(38 60 ff fc|fc ff 60 38) 	li      r3,-4
  20:	(38 60 00 04|04 00 60 38) 	li      r3,4
  24:	(38 60 ff fc|fc ff 60 38) 	li      r3,-4
  28:	(38 60 ff fc|fc ff 60 38) 	li      r3,-4
  2c:	(38 60 00 04|04 00 60 38) 	li      r3,4
  30:	(38 60 00 00|00 00 60 38) 	li      r3,0
			(32|30): R_PPC_ADDR16_LO	\.data
  34:	(38 60 00 00|00 00 60 38) 	li      r3,0
			(36|34): R_PPC_ADDR16_HI	\.data
  38:	(38 60 00 00|00 00 60 38) 	li      r3,0
			(3a|38): R_PPC_ADDR16_HA	\.data
  3c:	(38 60 ff fc|fc ff 60 38) 	li      r3,-4
  40:	(38 60 ff ff|ff ff 60 38) 	li      r3,-1
  44:	(38 60 00 00|00 00 60 38) 	li      r3,0
  48:	(80 64 00 04|04 00 64 80) 	lwz     r3,4\(r4\)
  4c:	(80 60 00 00|00 00 60 80) 	lwz     r3,0\(0\)
			(4e|4c): R_PPC_ADDR16_LO	\.text
Disassembly of section \.data:

0+0000 <dsym0>:
   0:	(de ad be ef|ef be ad de) 	stfdu   f21,-16657\(r13\)

0+0004 <dsym1>:
   4:	(ca fe ba be|be ba fe ca) 	lfd     f23,-17730\(r30\)

0+0008 <datpt>:
   8:	00 00 00 00 	\.long 0x0
			8: R_PPC_REL32	jk\+0x989680

0+000c <dat0>:
   c:	00 00 00 00 	\.long 0x0
			c: R_PPC_REL32	jk-0x4

0+0010 <dat1>:
  10:	00 00 00 00 	\.long 0x0
			10: R_PPC_REL32	jk

0+0014 <dat2>:
  14:	00 00 00 00 	\.long 0x0
			14: R_PPC_REL32	jk\+0x4
