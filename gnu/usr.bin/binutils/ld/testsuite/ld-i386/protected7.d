#as: --32
#ld: -shared -melf_i386
#objdump: -drw

.*: +file format .*


Disassembly of section .text:

0+[a-f0-9]+ <bar>:
[ 	]*[a-f0-9]+:	8b 81 [a-f0-9][a-f0-9] [a-f0-9][a-f0-9] 00 00    	mov    0x[a-f0-9]+\(%ecx\),%eax
[ 	]*[a-f0-9]+:	c3                   	ret
#pass
