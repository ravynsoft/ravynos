#as: -J
#source: ../x86-64-mpx-branch-1.s
#objdump: -dwr
#name: x86-64 MPX branch

.*: +file format .*


Disassembly of section .text:

0+ <foo1-0x1c>:
[ 	]*[a-f0-9]+:	f2 e8 00 00 00 00    	bnd call 6 <foo1-0x16>	2: R_X86_64_PC32	\*ABS\*\+0x10003c
[ 	]*[a-f0-9]+:	f2 e9 00 00 00 00    	bnd jmp c <foo1-0x10>	8: R_X86_64_PC32	\*ABS\*\+0x10003c
[ 	]*[a-f0-9]+:	66 f2 48 e8 00 00 00 00 	data16 bnd rex\.W call 14 <foo1-0x8>	10: R_X86_64_PC32	\*ABS\*\+0x10003c
[ 	]*[a-f0-9]+:	66 f2 48 e9 00 00 00 00 	data16 bnd rex\.W jmp 1c <foo1>	18: R_X86_64_PC32	\*ABS\*\+0x10003c

0+1c <foo1>:
[ 	]*[a-f0-9]+:	f2 eb fd             	bnd jmp 1c <foo1>
[ 	]*[a-f0-9]+:	f2 72 fa             	bnd jb 1c <foo1>
[ 	]*[a-f0-9]+:	f2 e8 f4 ff ff ff    	bnd call 1c <foo1>
[ 	]*[a-f0-9]+:	f2 eb 09             	bnd jmp 34 <foo2>
[ 	]*[a-f0-9]+:	f2 72 06             	bnd jb 34 <foo2>
[ 	]*[a-f0-9]+:	f2 e8 00 00 00 00    	bnd call 34 <foo2>

0+34 <foo2>:
[ 	]*[a-f0-9]+:	f2 e9 00 00 00 00    	bnd jmp 3a <foo2\+0x6>	36: R_X86_64_PC32	foo-0x4
[ 	]*[a-f0-9]+:	f2 0f 82 00 00 00 00 	bnd jb 41 <foo2\+0xd>	3d: R_X86_64_PC32	foo-0x4
[ 	]*[a-f0-9]+:	f2 e8 00 00 00 00    	bnd call 47 <foo2\+0x13>	43: R_X86_64_PC32	foo-0x4
[ 	]*[a-f0-9]+:	f2 e9 00 00 00 00    	bnd jmp 4d <foo2\+0x19>	49: R_X86_64_PLT32	foo-0x4
[ 	]*[a-f0-9]+:	f2 0f 82 00 00 00 00 	bnd jb 54 <foo2\+0x20>	50: R_X86_64_PLT32	foo-0x4
[ 	]*[a-f0-9]+:	f2 e8 00 00 00 00    	bnd call 5a <foo2\+0x26>	56: R_X86_64_PLT32	foo-0x4
