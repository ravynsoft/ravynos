#as: --32 -mrelax-relocations=yes
#ld: -shared -melf_i386 --version-script load7.map
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

#...
[ 	]*[a-f0-9]+:	8d 81 ([0-9a-f]{2} ){4} *	lea    -0x[a-f0-9]+\(%ecx\),%eax
[ 	]*[a-f0-9]+:	8d 81 ([0-9a-f]{2} ){4} *	lea    0x[a-f0-9]+\(%ecx\),%eax
[ 	]*[a-f0-9]+:	8d 81 ([0-9a-f]{2} ){4} *	lea    0x[a-f0-9]+\(%ecx\),%eax
[ 	]*[a-f0-9]+:	8d 81 ([0-9a-f]{2} ){4} *	lea    0x[a-f0-9]+\(%ecx\),%eax
#pass
