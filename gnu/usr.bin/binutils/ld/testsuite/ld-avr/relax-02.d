#name: AVR relaxation, symbol at end of section.
#as: -mmcu=avrxmega2 -mlink-relax
#ld:  -mavrxmega2 --relax
#source: relax-02.s
#objdump: -tzd
#target: avr-*-*

.*:     file format elf32-avr

SYMBOL TABLE:
#...
00000000 l     F \.text	0000000a local_start
0000000a l     F \.text	0000000a local_func_1
00000014 l     F \.text	0000000a local_func_2
0000001e l     F \.text	0000000a local_func_3
00000032 l       \.text	00000000 local_end_label
00000028 g       \.text	00000000 dest
#...
00000014 g     F \.text	0000000a func_2
#...
00000000 g     F \.text	0000000a _start
00000032 g       \.text	00000000 end_label
0000000a g     F \.text	0000000a func_1
#...
0000001e g     F \.text	0000000a func_3



Disassembly of section \.text:

00000000 <_start>:
   0:	00 00       	nop
   2:	00 00       	nop
   4:	00 00       	nop
   6:	00 00       	nop
   8:	00 00       	nop

0000000a <func_1>:
   a:	00 00       	nop
   c:	00 00       	nop
   e:	00 00       	nop
  10:	00 00       	nop
  12:	00 00       	nop

00000014 <func_2>:
  14:	00 00       	nop
  16:	08 c0       	rjmp	\.\+16     	; 0x28 <dest>
  18:	07 c0       	rjmp	\.\+14     	; 0x28 <dest>
  1a:	06 c0       	rjmp	\.\+12     	; 0x28 <dest>
  1c:	00 00       	nop

0000001e <func_3>:
  1e:	00 00       	nop
  20:	00 00       	nop
  22:	00 00       	nop
  24:	00 00       	nop
  26:	00 00       	nop

00000028 <dest>:
  28:	00 00       	nop
  2a:	00 00       	nop
  2c:	00 00       	nop
  2e:	00 00       	nop
  30:	00 00       	nop
