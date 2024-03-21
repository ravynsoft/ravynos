#objdump: -d
#name: loop_label
.*: +file format .*

Disassembly of section .text:

00000000 <.text>:
   0:	08 4f       	R0 <<= 0x1;
   2:	78 68       	P0 = 0xf \(X\);.*
   4:	48 42       	DIVS \(R0, R1\);
   6:	a2 e0 02 00 	LSETUP\(0x0xa, 0x0xa\) LC0 = P0;
   a:	08 42       	DIVQ \(R0, R1\);
   c:	80 42       	R0 = R0.L \(X\);
	...
