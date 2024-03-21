#as: -mrelax-relocations=yes
#objdump: -dwr

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	48 c7 c0 00 00 00 00 	mov    \$0x0,%rax	3: R_X86_64_GOT32	foo
[ 	]*[a-f0-9]+:	48 c7 c0 00 00 00 00 	mov    \$0x0,%rax	a: R_X86_64_GOT32	foo\+0x4
[ 	]*[a-f0-9]+:	48 c7 c0 00 00 00 00 	mov    \$0x0,%rax	11: R_X86_64_GOT32	foo
[ 	]*[a-f0-9]+:	48 c7 c0 00 00 00 00 	mov    \$0x0,%rax	18: R_X86_64_GOT32	foo\+0x4
#pass
