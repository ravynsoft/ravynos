#source: mov1.s
#as: --64 -mrelax-relocations=yes
#ld: -pie -melf_x86_64 --no-dynamic-linker
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

#...
[ 	]*[a-f0-9]+:	48 8b 05 ([0-9a-f]{2} ){4} *	mov    0x[a-f0-9]+\(%rip\),%rax        # [a-f0-9]+ <.*>
[ 	]*[a-f0-9]+:	48 c7 c0 00 00 00 00 *	mov    \$0x0,%rax
[ 	]*[a-f0-9]+:	48 c7 c0 00 00 00 00 *	mov    \$0x0,%rax
#pass
