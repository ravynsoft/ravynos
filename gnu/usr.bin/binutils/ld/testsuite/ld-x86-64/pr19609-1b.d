#source: pr19609-1.s
#as: --64 -mrelax-relocations=yes
#ld: -pie -melf_x86_64 --no-dynamic-linker
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

[a-f0-9]+ <_start>:
[ 	]*[a-f0-9]+:	48 81 f8 00 00 00 00 	cmp    \$0x0,%rax
[ 	]*[a-f0-9]+:	81 f9 00 00 00 00    	cmp    \$0x0,%ecx
[ 	]*[a-f0-9]+:	49 81 fb 00 00 00 00 	cmp    \$0x0,%r11
[ 	]*[a-f0-9]+:	41 81 fc 00 00 00 00 	cmp    \$0x0,%r12d
[ 	]*[a-f0-9]+:	48 c7 c0 00 00 00 00 	mov    \$0x0,%rax
[ 	]*[a-f0-9]+:	c7 c1 00 00 00 00    	mov    \$0x0,%ecx
[ 	]*[a-f0-9]+:	49 c7 c3 00 00 00 00 	mov    \$0x0,%r11
[ 	]*[a-f0-9]+:	41 c7 c4 00 00 00 00 	mov    \$0x0,%r12d
[ 	]*[a-f0-9]+:	48 f7 c0 00 00 00 00 	test   \$0x0,%rax
[ 	]*[a-f0-9]+:	f7 c1 00 00 00 00    	test   \$0x0,%ecx
[ 	]*[a-f0-9]+:	49 f7 c3 00 00 00 00 	test   \$0x0,%r11
[ 	]*[a-f0-9]+:	41 f7 c4 00 00 00 00 	test   \$0x0,%r12d
