#as: --x32
#ld: -m elf32_x86_64 -Ttext-segment 0xe0000000 -Ttext 0xe0010000
#objdump: -dw

.*: +file format elf32-x86-64.*


Disassembly of section .text:

e0010000 <_start>:
[ 	]*[a-f0-9]+:	48 b8 00 00 01 e0 00 00 00 00 	movabs \$0xe0010000,%rax
[ 	]*[a-f0-9]+:	48 a1 00 00 01 e0 00 00 00 00 	movabs 0xe0010000,%rax
#pass
