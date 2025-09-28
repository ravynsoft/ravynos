#objdump: -drwt
#name: x86-64 no-GOT

.*: +file format .*

SYMBOL TABLE:
0+ g     F .text	0000000000000000 foo
0+         \*UND\*	0000000000000000 bar



Disassembly of section .text:

0+ <foo>:
 +[a-f0-9]+:	e9 00 00 00 00       	jmp    5 <foo\+0x5>	1: R_X86_64_PLT32	bar-0x4
 +[a-f0-9]+:	e8 00 00 00 00       	call   a <foo\+0xa>	6: R_X86_64_PLT32	bar-0x4
#pass
