#source: mov1.s
#as: --32
#ld: -shared -melf_i386
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

#...
[ 	]*[a-f0-9]+:	8b 81 ([0-9a-f]{2} ){4} *	mov    -0x[a-f0-9]+\(%ecx\),%eax
[ 	]*[a-f0-9]+:	8b 81 ([0-9a-f]{2} ){4} *	mov    -0x[a-f0-9]+\(%ecx\),%eax
[ 	]*[a-f0-9]+:	8b 81 ([0-9a-f]{2} ){4} *	mov    -0x[a-f0-9]+\(%ecx\),%eax
#pass
