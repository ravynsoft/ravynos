#objdump: -drwt
#name: i386 no-GOT

.*: +file format .*

SYMBOL TABLE:
0+ g     F .text	00000000 foo
0+         \*UND\*	00000000 bar



Disassembly of section .text:

0+ <foo>:
 +[a-f0-9]+:	e9 fc ff ff ff       	jmp    1 <foo\+0x1>	1: R_386_PLT32	bar
 +[a-f0-9]+:	e8 fc ff ff ff       	call   6 <foo\+0x6>	6: R_386_PLT32	bar
#pass
