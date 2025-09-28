#objdump: -drx -Mpower4
#as: -mpower4 --generate-missing-build-notes=no
#name: Power4 instructions

.*
.*
architecture: powerpc:common64, flags 0x0+11:
HAS_RELOC, HAS_SYMS
start address 0x0+

Sections:
Idx Name +Size +VMA +LMA +File off +Algn
 +0 \.text +0+108 +0+ +0+ +.*
 +CONTENTS, ALLOC, LOAD, RELOC, READONLY, CODE
 +1 \.data +0+20 +0+ +0+ +.*
 +CONTENTS, ALLOC, LOAD, DATA
 +2 \.bss +0+ +0+ +0+ +.*
 +ALLOC
 +3 \.toc +0+20 +0+ +0+ +.*
 +CONTENTS, ALLOC, LOAD, RELOC, DATA
SYMBOL TABLE:
0+ l +d +\.text	0+ (|\.text)
0+ l +d +\.data	0+ (|\.data)
0+ l +d +\.bss	0+ (|\.bss)
0+ l +\.data	0+ dsym0
0+10 l +\.data	0+ dsym1
0+ l +d +\.toc	0+ (|\.toc)
0+10 l +\.data	0+ usym0
0+20 l +\.data	0+ usym1
0+ +\*UND\*	0+ esym0
0+ +\*UND\*	0+ esym1


Disassembly of section \.text:

0+ <\.text>:
.*:	(e0 83 00 00|00 00 83 e0) 	lq      r4,0\(r3\)
.*: R_PPC64_ADDR16_LO_DS	\.data
.*:	(e0 83 00 .0|.0 00 83 e0) 	lq      r4,.*\(r3\)
.*: R_PPC64_ADDR16_LO_DS	\.data\+0x10
.*:	(e0 83 00 .0|.0 00 83 e0) 	lq      r4,.*\(r3\)
.*: R_PPC64_ADDR16_LO_DS	\.data\+0x10
.*:	(e0 83 00 .0|.0 00 83 e0) 	lq      r4,.*\(r3\)
.*: R_PPC64_ADDR16_LO_DS	\.data\+0x20
.*:	(e0 83 00 00|00 00 83 e0) 	lq      r4,0\(r3\)
.*: R_PPC64_ADDR16_LO_DS	esym0
.*:	(e0 83 00 00|00 00 83 e0) 	lq      r4,0\(r3\)
.*: R_PPC64_ADDR16_LO_DS	esym1
.*:	(e0 82 00 00|00 00 82 e0) 	lq      r4,0\(r2\)
.*: R_PPC64_TOC16_DS	\.toc
.*:	(e0 82 00 .0|.0 00 82 e0) 	lq      r4,.*\(r2\)
.*: R_PPC64_TOC16_DS	\.toc\+0x10
.*:	(e0 80 00 00|00 00 80 e0) 	lq      r4,0\(0\)
.*: R_PPC64_ADDR16_LO_DS	\.text
.*:	(e0 c3 00 00|00 00 c3 e0) 	lq      r6,0\(r3\)
.*: R_PPC64_GOT16_DS	dsym0
.*:	(e0 c3 00 00|00 00 c3 e0) 	lq      r6,0\(r3\)
.*: R_PPC64_GOT16_LO_DS	dsym0
.*:	(e0 c3 00 00|00 00 c3 e0) 	lq      r6,0\(r3\)
.*: R_PPC64_PLT16_LO_DS	dsym0
.*:	(e0 c3 00 .0|.0 00 c3 e0) 	lq      r6,.*\(r3\)
.*: R_PPC64_SECTOFF_DS	\.data\+0x10
.*:	(e0 c3 00 .0|.0 00 c3 e0) 	lq      r6,.*\(r3\)
.*: R_PPC64_SECTOFF_LO_DS	\.data\+0x10
.*:	(e0 c4 00 20|20 00 c4 e0) 	lq      r6,32\(r4\)
.*:	(f8 c7 00 02|02 00 c7 f8) 	stq     r6,0\(r7\)
.*:	(f8 c7 00 12|12 00 c7 f8) 	stq     r6,16\(r7\)
.*:	(f8 c7 ff f2|f2 ff c7 f8) 	stq     r6,-16\(r7\)
.*:	(f8 c7 80 02|02 80 c7 f8) 	stq     r6,-32768\(r7\)
.*:	(f8 c7 7f f2|f2 7f c7 f8) 	stq     r6,32752\(r7\)
.*:	(00 00 02 00|00 02 00 00) 	attn
.*:	(7c 6f f1 20|20 f1 6f 7c) 	mtcr    r3
.*:	(7c 6f f1 20|20 f1 6f 7c) 	mtcr    r3
.*:	(7c 68 11 20|20 11 68 7c) 	mtcrf   129,r3
.*:	(7c 70 11 20|20 11 70 7c) 	mtocrf  1,r3
.*:	(7c 70 21 20|20 21 70 7c) 	mtocrf  2,r3
.*:	(7c 70 41 20|20 41 70 7c) 	mtocrf  4,r3
.*:	(7c 70 81 20|20 81 70 7c) 	mtocrf  8,r3
.*:	(7c 71 01 20|20 01 71 7c) 	mtocrf  16,r3
.*:	(7c 72 01 20|20 01 72 7c) 	mtocrf  32,r3
.*:	(7c 74 01 20|20 01 74 7c) 	mtocrf  64,r3
.*:	(7c 78 01 20|20 01 78 7c) 	mtocrf  128,r3
.*:	(7c 60 00 26|26 00 60 7c) 	mfcr    r3
.*:	(7c 70 10 26|26 10 70 7c) 	mfocrf  r3,1
.*:	(7c 70 20 26|26 20 70 7c) 	mfocrf  r3,2
.*:	(7c 70 40 26|26 40 70 7c) 	mfocrf  r3,4
.*:	(7c 70 80 26|26 80 70 7c) 	mfocrf  r3,8
.*:	(7c 71 00 26|26 00 71 7c) 	mfocrf  r3,16
.*:	(7c 72 00 26|26 00 72 7c) 	mfocrf  r3,32
.*:	(7c 74 00 26|26 00 74 7c) 	mfocrf  r3,64
.*:	(7c 78 00 26|26 00 78 7c) 	mfocrf  r3,128
.*:	(7c 01 17 ec|ec 17 01 7c) 	dcbz    r1,r2
.*:	(7c 23 27 ec|ec 27 23 7c) 	dcbzl   r3,r4
.*:	(7c 05 37 ec|ec 37 05 7c) 	dcbz    r5,r6
.*:	(e0 40 00 10|10 00 40 e0) 	lq      r2,16\(0\)
.*:	(e0 05 00 10|10 00 05 e0) 	lq      r0,16\(r5\)
.*:	(e0 45 00 10|10 00 45 e0) 	lq      r2,16\(r5\)
.*:	(f8 40 00 12|12 00 40 f8) 	stq     r2,16\(0\)
.*:	(f8 05 00 12|12 00 05 f8) 	stq     r0,16\(r5\)
.*:	(f8 45 00 12|12 00 45 f8) 	stq     r2,16\(r5\)
.*:	(7c 00 03 e4|e4 03 00 7c) 	slbia
.*:	(7c 00 04 ac|ac 04 00 7c) 	hwsync
.*:	(7c 00 04 ac|ac 04 00 7c) 	hwsync
.*:	(7c 00 04 ac|ac 04 00 7c) 	hwsync
.*:	(7c 20 04 ac|ac 04 20 7c) 	lwsync
.*:	(7c 20 04 ac|ac 04 20 7c) 	lwsync
.*:	(7c 40 04 ac|ac 04 40 7c) 	ptesync
.*:	(7c 40 04 ac|ac 04 40 7c) 	ptesync
.*:	(7e 80 30 28|28 30 80 7e) 	lwarx   r20,0,r6
.*:	(7e 81 30 28|28 30 81 7e) 	lwarx   r20,r1,r6
.*:	(7e a0 38 a8|a8 38 a0 7e) 	ldarx   r21,0,r7
.*:	(7e a1 38 a8|a8 38 a1 7e) 	ldarx   r21,r1,r7
.*:	(7e c0 41 2d|2d 41 c0 7e) 	stwcx\.  r22,0,r8
.*:	(7e c1 41 2d|2d 41 c1 7e) 	stwcx\.  r22,r1,r8
.*:	(7e e0 49 ad|ad 49 e0 7e) 	stdcx\.  r23,0,r9
.*:	(7e e1 49 ad|ad 49 e1 7e) 	stdcx\.  r23,r1,r9
#pass
