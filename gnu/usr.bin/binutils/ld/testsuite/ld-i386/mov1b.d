#source: mov1.s
#as: --32 -mrelax-relocations=yes
#ld: -pie -melf_i386 --no-dynamic-linker
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

#...
[ 	]*[a-f0-9]+:	8b 81 ([0-9a-f]{2} ){4} *	mov    -0x[a-f0-9]+\(%ecx\),%eax
[ 	]*[a-f0-9]+:	c7 c0 00 00 00 00 *	mov    \$0x0,%eax
[ 	]*[a-f0-9]+:	c7 c0 00 00 00 00 *	mov    \$0x0,%eax
#pass
