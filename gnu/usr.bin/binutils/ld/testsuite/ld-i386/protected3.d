#as: --32
#ld: -shared -melf_i386
#objdump: -drw

.*: +file format .*


Disassembly of section .text:

0+[a-f0-9]+ <bar>:
[ 	]*[a-f0-9]+:	8d 81 00 00 00 00    	lea    0x0\(%ecx\),%eax
[ 	]*[a-f0-9]+:	8b 00                	mov    \(%eax\),%eax
[ 	]*[a-f0-9]+:	c3                   	ret
#pass
