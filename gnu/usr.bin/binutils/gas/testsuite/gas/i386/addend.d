#as: -mrelax-relocations=yes
#objdump: -dwr

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	b8 00 00 00 00       	mov    \$0x0,%eax	1: R_386_GOT32	foo
[ 	]*[a-f0-9]+:	b8 04 00 00 00       	mov    \$0x4,%eax	6: R_386_GOT32	foo
[ 	]*[a-f0-9]+:	b8 00 00 00 00       	mov    \$0x0,%eax	b: R_386_GOT32	foo
[ 	]*[a-f0-9]+:	b8 04 00 00 00       	mov    \$0x4,%eax	10: R_386_GOT32	foo
#pass
