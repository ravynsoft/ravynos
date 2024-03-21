#source: mov1.s
#as: --x32 -mrelax-relocations=yes
#ld: -pie -melf32_x86_64 --no-dynamic-linker
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

#...
[ 	]*[a-f0-9]+:	48 8b 05 ([0-9a-f]{2} ){4} *	mov    0x[a-f0-9]+\(%rip\),%rax        # [a-f0-9]+ <.*>
[ 	]*[a-f0-9]+:	40 c7 c0 00 00 00 00 *	rex mov \$0x0,%eax
[ 	]*[a-f0-9]+:	40 c7 c0 00 00 00 00 *	rex mov \$0x0,%eax
#pass
