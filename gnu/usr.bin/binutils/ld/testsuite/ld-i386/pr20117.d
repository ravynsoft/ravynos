#as: --32
#ld: -melf_i386
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

[a-f0-9]+ <_start>:
[ 	]*[a-f0-9]+:	eb 8b                	jmp    [a-f0-9]+ <_start\-0x[a-f0-9]+>
[ 	]*[a-f0-9]+:	bd ([0-9a-f]{2} ){4} *	mov    \$0x[a-f0-9]+\,%ebp
