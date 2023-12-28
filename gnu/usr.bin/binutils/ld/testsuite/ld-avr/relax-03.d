#name: AVR relaxation, single function in section.
#as: -mmcu=avrxmega2 -mlink-relax
#ld:  -mavrxmega2 --relax
#source: relax-03.s
#objdump: -tzd
#target: avr-*-*

.*:     file format elf32-avr

SYMBOL TABLE:
#...
00000000 l     F .text	0000000a local_start
0000000a l       .text	00000000 local_end_label
#...
00000000 g     F \.text	0000000a _start
0000000a g       \.text	00000000 end_label
#...

Disassembly of section \.text:

00000000 <_start>:
   0:	00 00       	nop
   2:	03 c0       	rjmp	\.\+6      	; 0xa <.*>
   4:	02 c0       	rjmp	\.\+4      	; 0xa <.*>
   6:	01 c0       	rjmp	\.\+2      	; 0xa <.*>
   8:	00 00       	nop
