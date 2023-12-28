#source: pcrel.s
#as: -mshared
#objdump: -drw
#name: i386 pcrel ELF reloc

.*: +file format .*i386.*

Disassembly of section \.text:

0+ <loc>:
[ 	]*[a-f0-9]+:	e9 30 12 00 00       	jmp    1235 <abs\+0x1>	1: R_386_PC32	\*ABS\*

0+5 <glob>:
[ 	]*[a-f0-9]+:	e9 fc ff ff ff       	jmp    6 <glob\+0x1>	6: R_386_PC32	ext
[ 	]*[a-f0-9]+:	e9 fc ff ff ff       	jmp    b <glob\+0x6>	b: R_386_PC32	weak
[ 	]*[a-f0-9]+:	e9 fc ff ff ff       	jmp    10 <glob\+0xb>	10: R_386_PC32	comm
[ 	]*[a-f0-9]+:	eb ea                	jmp    0 <loc>
[ 	]*[a-f0-9]+:	e9 fc ff ff ff       	jmp    17 <glob\+0x12>	17: R_386_PC32	glob
[ 	]*[a-f0-9]+:	e9 72 98 00 00       	jmp    9892 <abs2\+0x1c>	1c: R_386_PC32	\*ABS\*
[ 	]*[a-f0-9]+:	e9 db 00 00 00       	jmp    100 <loc2>
[ 	]*[a-f0-9]+:	e9 fc ff ff ff       	jmp    26 <glob\+0x21>	26: R_386_PC32	glob2
[ 	]*[a-f0-9]+:	e9 fc ff ff ff       	jmp    2b <glob\+0x26>	2b: R_386_PC32	.data
[ 	]*[a-f0-9]+:	e9 00 00 00 00       	jmp    34 <glob\+0x2f>	30: R_386_PC32	.data
[ 	]*[a-f0-9]+:	e9 fc ff ff ff       	jmp    35 <glob\+0x30>	35: R_386_PC32	\*ABS\*
[ 	]*[a-f0-9]+:	e9 c8 ed ff ff       	jmp    ffffee06 <abs2\+0xffff5590>	3a: R_386_PC32	ext
[ 	]*[a-f0-9]+:	e9 c8 ed ff ff       	jmp    ffffee0b <abs2\+0xffff5595>	3f: R_386_PC32	weak
[ 	]*[a-f0-9]+:	e9 c8 ed ff ff       	jmp    ffffee10 <abs2\+0xffff559a>	44: R_386_PC32	comm
[ 	]*[a-f0-9]+:	e9 7f ed ff ff       	jmp    ffffedcc <abs2\+0xffff5556>
[ 	]*[a-f0-9]+:	e9 c8 ed ff ff       	jmp    ffffee1a <abs2\+0xffff55a4>	4e: R_386_PC32	glob
[ 	]*[a-f0-9]+:	e9 3e 86 00 00       	jmp    8695 <abs\+0x7461>	53: R_386_PC32	\*ABS\*
[ 	]*[a-f0-9]+:	e9 70 ee ff ff       	jmp    ffffeecc <abs2\+0xffff5656>
[ 	]*[a-f0-9]+:	e9 c8 ed ff ff       	jmp    ffffee29 <abs2\+0xffff55b3>	5d: R_386_PC32	glob2
[ 	]*[a-f0-9]+:	e9 c8 ed ff ff       	jmp    ffffee2e <abs2\+0xffff55b8>	62: R_386_PC32	.data
[ 	]*[a-f0-9]+:	e9 cc ed ff ff       	jmp    ffffee37 <abs2\+0xffff55c1>	67: R_386_PC32	.data
[ 	]*[a-f0-9]+:	e9 ba 79 ff ff       	jmp    ffff7a2a <abs2\+0xfffee1b4>	6c: R_386_PC32	\*ABS\*
[ 	]*[a-f0-9]+:	e9 86 67 ff ff       	jmp    ffff67fb <abs2\+0xfffecf85>	71: R_386_PC32	ext
[ 	]*[a-f0-9]+:	e9 86 67 ff ff       	jmp    ffff6800 <abs2\+0xfffecf8a>	76: R_386_PC32	weak
[ 	]*[a-f0-9]+:	e9 86 67 ff ff       	jmp    ffff6805 <abs2\+0xfffecf8f>	7b: R_386_PC32	comm
[ 	]*[a-f0-9]+:	e9 06 67 ff ff       	jmp    ffff678a <abs2\+0xfffecf14>
[ 	]*[a-f0-9]+:	e9 06 67 ff ff       	jmp    ffff678f <abs2\+0xfffecf19>
[ 	]*[a-f0-9]+:	e9 fc ff ff ff       	jmp    8a <glob\+0x85>	8a: R_386_PC32	\*ABS\*
[ 	]*[a-f0-9]+:	e9 f7 67 ff ff       	jmp    ffff688a <abs2\+0xfffed014>
[ 	]*[a-f0-9]+:	e9 f7 67 ff ff       	jmp    ffff688f <abs2\+0xfffed019>
[ 	]*[a-f0-9]+:	e9 86 67 ff ff       	jmp    ffff6823 <abs2\+0xfffecfad>	99: R_386_PC32	.data
[ 	]*[a-f0-9]+:	e9 8a 67 ff ff       	jmp    ffff682c <abs2\+0xfffecfb6>	9e: R_386_PC32	.data
[ 	]*[a-f0-9]+:	e9 fc 00 00 00       	jmp    1a3 <glob2\+0x9e>	a3: R_386_PC32	\*ABS\*
[ 	]*[a-f0-9]+:	e9 01 00 00 00       	jmp    ad <glob\+0xa8>	a8: R_386_PC32	\*ABS\*
[ 	]*[a-f0-9]+:	e9 01 ff ff ff       	jmp    ffffffb2 <abs2\+0xffff673c>	ad: R_386_PC32	\*ABS\*
[ 	]*[a-f0-9]+:	e9 01 01 00 00       	jmp    1b7 <glob2\+0xb2>	b2: R_386_PC32	\*ABS\*
[ 	]*[a-f0-9]+:	e9 01 00 00 00       	jmp    bc <glob\+0xb7>	b7: R_386_PC32	\*ABS\*
	...
#pass
