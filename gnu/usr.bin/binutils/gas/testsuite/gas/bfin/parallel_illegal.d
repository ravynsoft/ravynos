#objdump: -dr
#name: parallel_illegal
.*: +file format .*

Disassembly of section .text:

00000000 <.text>:
   0:	03 c8 00 18 	MNOP || NOP || NOP;
   4:	00 00 00 00 
   8:	03 c8       	MNOP || ILLEGAL || NOP;.*
   a:	00 18       	IF CC JUMP 0x0xa;
   c:	01 00       	ILLEGAL;
   e:	00 00       	NOP;
  10:	03 c8       	MNOP || NOP || ILLEGAL;.*
  12:	00 18       	IF CC JUMP 0x0x12;
  14:	00 00       	NOP;
  16:	01 00       	ILLEGAL;
  18:	03 c8       	MNOP || ILLEGAL || ILLEGAL;.*
  1a:	00 18       	IF CC JUMP 0x0x1a;
  1c:	01 00       	ILLEGAL;
  1e:	01 00       	ILLEGAL;
  20:	03 c8       	MNOP || ILLEGAL || ILLEGAL;.*
  22:	00 18       	IF CC JUMP 0x0x22;
  24:	10 00       	RTS;
  26:	10 00       	RTS;
  28:	03 c8       	MNOP || ILLEGAL || ILLEGAL;.*
  2a:	00 18       	IF CC JUMP 0x0x2a;
  2c:	03 c0 00 18 	MNOP;
  30:	00 00       	NOP;
	...
