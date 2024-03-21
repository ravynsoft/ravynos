#source: ../relax-3.s
#as: -mshared
#objdump: -dwr

.*: +file format .*


Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	eb 24                	jmp    26 <local>
[ 	]*[a-f0-9]+:	eb 1e                	jmp    22 <hidden_def>
[ 	]*[a-f0-9]+:	e9 00 00 00 00       	jmp    9 <foo\+0x9>	5: R_X86_64_PC32	global_def-0x4
[ 	]*[a-f0-9]+:	e9 00 00 00 00       	jmp    e <foo\+0xe>	a: R_X86_64_PLT32	global_def-0x4
[ 	]*[a-f0-9]+:	e9 00 00 00 00       	jmp    13 <foo\+0x13>	f: R_X86_64_PC32	weak_def-0x4
[ 	]*[a-f0-9]+:	e9 00 00 00 00       	jmp    18 <foo\+0x18>	14: R_X86_64_PC32	weak_hidden_undef-0x4
[ 	]*[a-f0-9]+:	e9 00 00 00 00       	jmp    1d <foo\+0x1d>	19: R_X86_64_PC32	weak_hidden_def-0x4
[ 	]*[a-f0-9]+:	e9 00 00 00 00       	jmp    22 <hidden_def>	1e: R_X86_64_PC32	hidden_undef-0x4

0+22 <hidden_def>:
[ 	]*[a-f0-9]+:	c3                   	ret *

0+23 <weak_hidden_def>:
[ 	]*[a-f0-9]+:	c3                   	ret *

0+24 <global_def>:
[ 	]*[a-f0-9]+:	c3                   	ret *

0+25 <weak_def>:
[ 	]*[a-f0-9]+:	c3                   	ret *

0+26 <local>:
[ 	]*[a-f0-9]+:	c3                   	ret *
#pass
