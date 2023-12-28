#source: pr19609-1.s
#as: --32 -mrelax-relocations=yes
#ld: -melf_i386
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

[a-f0-9]+ <_start>:
[ 	]*[a-f0-9]+:	81 f8 00 00 00 00    	cmp    \$0x0,%eax
[ 	]*[a-f0-9]+:	81 f9 00 00 00 00    	cmp    \$0x0,%ecx
[ 	]*[a-f0-9]+:	c7 c0 00 00 00 00    	mov    \$0x0,%eax
[ 	]*[a-f0-9]+:	c7 c1 00 00 00 00    	mov    \$0x0,%ecx
[ 	]*[a-f0-9]+:	f7 c0 00 00 00 00    	test   \$0x0,%eax
[ 	]*[a-f0-9]+:	f7 c1 00 00 00 00    	test   \$0x0,%ecx
