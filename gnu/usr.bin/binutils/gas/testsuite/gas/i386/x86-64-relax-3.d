#source: relax-3.s
#objdump: -dwr
#notarget: *-*-solaris*

.*: +file format .*


Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	eb 21                	jmp    23 <local>
[ 	]*[a-f0-9]+:	eb 1b                	jmp    1f <hidden_def>
[ 	]*[a-f0-9]+:	eb 1b                	jmp    21 <global_def>
[ 	]*[a-f0-9]+:	e9 00 00 00 00       	jmp    b <foo\+0xb>	7: R_X86_64_PLT32	global_def-0x4
[ 	]*[a-f0-9]+:	e9 00 00 00 00       	jmp    10 <foo\+0x10>	c: R_X86_64_PLT32	weak_def-0x4
[ 	]*[a-f0-9]+:	e9 00 00 00 00       	jmp    15 <foo\+0x15>	11: R_X86_64_PLT32	weak_hidden_undef-0x4
[ 	]*[a-f0-9]+:	e9 00 00 00 00       	jmp    1a <foo\+0x1a>	16: R_X86_64_PLT32	weak_hidden_def-0x4
[ 	]*[a-f0-9]+:	e9 00 00 00 00       	jmp    1f <hidden_def>	1b: R_X86_64_PLT32	hidden_undef-0x4

0+1f <hidden_def>:
[ 	]*[a-f0-9]+:	c3                   	ret

0+20 <weak_hidden_def>:
[ 	]*[a-f0-9]+:	c3                   	ret

0+21 <global_def>:
[ 	]*[a-f0-9]+:	c3                   	ret

0+22 <weak_def>:
[ 	]*[a-f0-9]+:	c3                   	ret

0+23 <local>:
[ 	]*[a-f0-9]+:	c3                   	ret
#pass
