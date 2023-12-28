#as:
#objdump: -dr
#name: immed-1

.*: +file format .*

Disassembly of section .text:

0+0000000 <bar>:
       0:	04 c1 00 00 	moviq   r1,0
       4:	84 41 00 0c 	subi    r1,12
       8:	04 01 00 00 	addi    r1,0
			8: R_VISIUM_IM16	\.text
       c:	84 c4 ff ec 	moviq   r4,65516
	\.\.\.
    1010:	04 c6 00 08 	moviq   r6,8
    1014:	04 c7 00 e4 	moviq   r7,228
