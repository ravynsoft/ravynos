#objdump: -Drx
#as:  --generate-missing-build-notes=no
#name: PowerPC Test 1, 64 bit elf

.*
.*
architecture: powerpc:common64, flags 0x00000011:
HAS_RELOC, HAS_SYMS
start address 0x0000000000000000

Sections:
Idx Name          Size      VMA               LMA               File off  Algn
  0 \.text         00000090  0000000000000000  0000000000000000  .*
                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, CODE
  1 \.data         00000030  0000000000000000  0000000000000000  .*
                  CONTENTS, ALLOC, LOAD, RELOC, DATA
  2 \.bss          00000000  0000000000000000  0000000000000000  .*
                  ALLOC
  3 \.toc          00000030  0000000000000000  0000000000000000  .*
                  CONTENTS, ALLOC, LOAD, RELOC, DATA
SYMBOL TABLE:
0000000000000000 l    d  \.text	0000000000000000 (|\.text)
0000000000000000 l    d  \.data	0000000000000000 (|\.data)
0000000000000000 l    d  \.bss	0000000000000000 (|\.bss)
0000000000000000 l       \.data	0000000000000000 dsym0
0000000000000008 l       \.data	0000000000000000 dsym1
0000000000000000 l    d  \.toc	0000000000000000 (|\.toc)
0000000000000008 l       \.data	0000000000000000 usym0
0000000000000010 l       \.data	0000000000000000 usym1
0000000000000010 l       \.data	0000000000000000 datpt
0000000000000014 l       \.data	0000000000000000 dat0
0000000000000018 l       \.data	0000000000000000 dat1
000000000000001c l       \.data	0000000000000000 dat2
0000000000000020 l       \.data	0000000000000000 dat3
0000000000000028 l       \.data	0000000000000000 dat4
0000000000000000         \*UND\*	0000000000000000 esym0
0000000000000000         \*UND\*	0000000000000000 esym1
0000000000000000         \*UND\*	0000000000000000 jk


Disassembly of section \.text:

0000000000000000 <\.text>:
   0:	(e8 63 00 00|00 00 63 e8) 	ld      r3,0\(r3\)
			(2|0): R_PPC64_ADDR16_LO_DS	\.data
   4:	(e8 63 00 0.|0. 00 63 e8) 	ld      r3,.\(r3\)
			(6|4): R_PPC64_ADDR16_LO_DS	\.data\+0x8
   8:	(e8 63 00 0.|0. 00 63 e8) 	ld      r3,.\(r3\)
			(a|8): R_PPC64_ADDR16_LO_DS	\.data\+0x8
   c:	(e8 63 00 .0|.0 00 63 e8) 	ld      r3,.*\(r3\)
			(e|c): R_PPC64_ADDR16_LO_DS	\.data\+0x10
  10:	(e8 63 00 00|00 00 63 e8) 	ld      r3,0\(r3\)
			1(0|2): R_PPC64_ADDR16_LO_DS	esym0
  14:	(e8 63 00 00|00 00 63 e8) 	ld      r3,0\(r3\)
			1(6|4): R_PPC64_ADDR16_LO_DS	esym1
  18:	(e8 62 00 00|00 00 62 e8) 	ld      r3,0\(r2\)
			1(a|8): R_PPC64_TOC16_DS	\.toc
  1c:	(e8 62 00 0.|0. 00 62 e8) 	ld      r3,.\(r2\)
			1(e|c): R_PPC64_TOC16_DS	\.toc\+0x8
  20:	(e8 62 00 .0|.0 00 62 e8) 	ld      r3,.*\(r2\)
			2(2|0): R_PPC64_TOC16_DS	\.toc\+0x10
  24:	(e8 62 00 ..|.. 00 62 e8) 	ld      r3,.*\(r2\)
			2(6|4): R_PPC64_TOC16_DS	\.toc\+0x18
  28:	(e8 62 00 .0|.0 00 62 e8) 	ld      r3,.*\(r2\)
			2(a|8): R_PPC64_TOC16_DS	\.toc\+0x20
  2c:	(e8 62 00 ..|.. 00 62 e8) 	ld      r3,.*\(r2\)
			2(e|c): R_PPC64_TOC16_DS	\.toc\+0x28
  30:	(3c 80 00 ..|.. 00 80 3c) 	lis     r4,.*
			3(2|0): R_PPC64_TOC16_HA	\.toc\+0x28
  34:	(e8 62 00 ..|.. 00 62 e8) 	ld      r3,.*\(r2\)
			3(6|4): R_PPC64_TOC16_LO_DS	\.toc\+0x28
  38:	(38 60 00 08|08 00 60 38) 	li      r3,8
  3c:	(38 60 ff f8|f8 ff 60 38) 	li      r3,-8
  40:	(38 60 00 08|08 00 60 38) 	li      r3,8
  44:	(38 60 ff f8|f8 ff 60 38) 	li      r3,-8
  48:	(38 60 ff f8|f8 ff 60 38) 	li      r3,-8
  4c:	(38 60 00 08|08 00 60 38) 	li      r3,8
  50:	(38 60 00 00|00 00 60 38) 	li      r3,0
			5(2|0): R_PPC64_ADDR16_LO	\.data
  54:	(38 60 00 00|00 00 60 38) 	li      r3,0
			5(6|4): R_PPC64_ADDR16_HI	\.data
  58:	(38 60 00 00|00 00 60 38) 	li      r3,0
			5(a|8): R_PPC64_ADDR16_HA	\.data
  5c:	(38 60 00 00|00 00 60 38) 	li      r3,0
			5(e|c): R_PPC64_ADDR16_HIGHER	\.data
  60:	(38 60 00 00|00 00 60 38) 	li      r3,0
			6(2|0): R_PPC64_ADDR16_HIGHERA	\.data
  64:	(38 60 00 00|00 00 60 38) 	li      r3,0
			6(6|4): R_PPC64_ADDR16_HIGHEST	\.data
  68:	(38 60 00 00|00 00 60 38) 	li      r3,0
			6(a|8): R_PPC64_ADDR16_HIGHESTA	\.data
  6c:	(38 60 ff f8|f8 ff 60 38) 	li      r3,-8
  70:	(38 60 ff ff|ff ff 60 38) 	li      r3,-1
  74:	(38 60 00 00|00 00 60 38) 	li      r3,0
  78:	(38 60 ff ff|ff ff 60 38) 	li      r3,-1
  7c:	(38 60 00 00|00 00 60 38) 	li      r3,0
  80:	(38 60 ff ff|ff ff 60 38) 	li      r3,-1
  84:	(38 60 00 00|00 00 60 38) 	li      r3,0
  88:	(e8 64 00 08|08 00 64 e8) 	ld      r3,8\(r4\)
  8c:	(e8 60 00 00|00 00 60 e8) 	ld      r3,0\(0\)
			8(e|c): R_PPC64_ADDR16_LO_DS	\.text
Disassembly of section \.data:

0000000000000000 <dsym0>:
   0:	(00 00 00 00|ef be ad de) .*
   4:	(de ad be ef|00 00 00 00) .*

0000000000000008 <dsym1>:
   8:	(00 00 00 00|be ba fe ca) .*
   c:	(ca fe ba be|00 00 00 00) .*

0000000000000010 <datpt>:
  10:	00 00 00 00 .*
			10: R_PPC64_REL32	jk\+0x989680

0000000000000014 <dat0>:
  14:	00 00 00 00 .*
			14: R_PPC64_REL32	jk-0x4

0000000000000018 <dat1>:
  18:	00 00 00 00 .*
			18: R_PPC64_REL32	jk

000000000000001c <dat2>:
  1c:	00 00 00 00 .*
			1c: R_PPC64_REL32	jk\+0x4

0000000000000020 <dat3>:
	\.\.\.
			20: R_PPC64_REL64	jk\+0x8

0000000000000028 <dat4>:
	\.\.\.
			28: R_PPC64_REL64	jk\+0x10
Disassembly of section \.toc:

0000000000000000 <\.toc>:
#...
			0: R_PPC64_ADDR64	\.data
#...
			8: R_PPC64_ADDR64	\.data\+0x8
#...
			10: R_PPC64_ADDR64	\.data\+0x8
#...
			18: R_PPC64_ADDR64	\.data\+0x10
#...
			20: R_PPC64_ADDR64	esym0
			28: R_PPC64_ADDR64	esym1
