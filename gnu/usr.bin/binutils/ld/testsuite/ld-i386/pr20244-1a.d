#source: pr20244-1.s
#as: --32
#ld: -m elf_i386 -z noseparate-code
#objdump: --sym -dw

.*: +file format .*

SYMBOL TABLE:
#...
0+80490a0 l     O .data	00000001 bar
#...
0+8048074 g     F .text	00000000 _start
#...
0+80490a1 g     O .data	00000001 foo
#...



Disassembly of section .text:

0+8048074 <_start>:
 +[a-f0-9]+:	c7 05 8c 90 04 08 00 00 00 00 	movl   \$0x0,0x804908c
 +[a-f0-9]+:	83 3d 90 90 04 08 00 	cmpl   \$0x0,0x8049090
 +[a-f0-9]+:	b9 f8 ff ff ff       	mov    \$0xfffffff8,%ecx
#pass
