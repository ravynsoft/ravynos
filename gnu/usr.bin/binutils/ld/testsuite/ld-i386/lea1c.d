#source: lea1.s
#as: --32 -mrelax-relocations=yes
#ld: -melf_i386
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

#...
[ 	]*[a-f0-9]+:	c7 c0 ([0-9a-f]{2} ){4} *	mov    \$0x[a-f0-9]+,%eax
[ 	]*[a-f0-9]+:	c7 c0 ([0-9a-f]{2} ){4} *	mov    \$0x[a-f0-9]+,%eax
[ 	]*[a-f0-9]+:	c7 c0 ([0-9a-f]{2} ){4} *	mov    \$0x[a-f0-9]+,%eax
[ 	]*[a-f0-9]+:	c7 c0 ([0-9a-f]{2} ){4} *	mov    \$0x[a-f0-9]+,%eax
#pass
