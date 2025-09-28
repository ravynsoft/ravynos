#objdump: -d
#name: flow2
.*: +file format .*

Disassembly of section .text:

00000000 <MY_LABEL1-0x2a>:
   0:	50 00       	JUMP \(P0\);
   2:	51 00       	JUMP \(P1\);
   4:	52 00       	JUMP \(P2\);
   6:	53 00       	JUMP \(P3\);
   8:	54 00       	JUMP \(P4\);
   a:	55 00       	JUMP \(P5\);
   c:	56 00       	JUMP \(SP\);
   e:	57 00       	JUMP \(FP\);
  10:	80 00       	JUMP \(PC \+ P0\);
  12:	81 00       	JUMP \(PC \+ P1\);
  14:	82 00       	JUMP \(PC \+ P2\);
  16:	83 00       	JUMP \(PC \+ P3\);
  18:	84 00       	JUMP \(PC \+ P4\);
  1a:	85 00       	JUMP \(PC \+ P5\);
  1c:	86 00       	JUMP \(PC \+ SP\);
  1e:	87 00       	JUMP \(PC \+ FP\);
  20:	00 20       	JUMP.S 0x20 <MY_LABEL1-0xa>;
  22:	69 22       	JUMP.S 0x4f4 .*
  24:	97 2d       	JUMP.S 0xfffffb52 .*
  26:	01 20       	JUMP.S 0x28 <MY_LABEL1-0x2>;
  28:	ff 2f       	JUMP.S 0x26 <MY_LABEL1-0x4>;

0000002a <MY_LABEL1>:
  2a:	00 20       	JUMP.S 0x2a <MY_LABEL1>;
  2c:	69 22       	JUMP.S 0x4fe .*
  2e:	97 2d       	JUMP.S 0xfffffb5c .*
  30:	01 20       	JUMP.S 0x32 <MY_LABEL1\+0x8>;
  32:	ff 2f       	JUMP.S 0x30 <MY_LABEL1\+0x6>;
  34:	c0 e2 00 00 	JUMP.L 0xff800034 .*
  38:	3f e2 ff ff 	JUMP.L 0x800036 .*
  3c:	00 e2 00 00 	JUMP.L 0x3c <MY_LABEL1\+0x12>;
  40:	00 e2 69 02 	JUMP.L 0x512 .*
  44:	ff e2 97 fd 	JUMP.L 0xfffffb72 .*
  48:	00 e2 01 00 	JUMP.L 0x4a <MY_LABEL1\+0x20>;
  4c:	ff e2 ff ff 	JUMP.L 0x4a <MY_LABEL1\+0x20>;
  50:	ed 2f       	JUMP.S 0x2a <MY_LABEL1>;
  52:	d7 2f       	JUMP.S 0x0 .*
  54:	d6 2f       	JUMP.S 0x0 .*
  56:	d5 2f       	JUMP.S 0x0 .*
  58:	04 1b       	IF CC JUMP 0xfffffe60 .*
  5a:	5a 18       	IF CC JUMP 0x10e .*
  5c:	00 18       	IF CC JUMP 0x5c <MY_LABEL1\+0x32>;
  5e:	04 1f       	IF CC JUMP 0xfffffe66 .*\(BP\);
  60:	5a 1c       	IF CC JUMP 0x114 .*\(BP\);
  62:	91 13       	IF !CC JUMP 0xffffff84 .*;
  64:	90 10       	IF !CC JUMP 0x184 .*;
  66:	91 17       	IF !CC JUMP 0xffffff88 .*\(BP\);
  68:	90 14       	IF !CC JUMP 0x188 .*\(BP\);
  6a:	e0 1b       	IF CC JUMP 0x2a <MY_LABEL1>;
  6c:	ca 1b       	IF CC JUMP 0x0 <MY_LABEL1-0x2a>;
  6e:	de 1f       	IF CC JUMP 0x2a <MY_LABEL1> \(BP\);
  70:	c8 1f       	IF CC JUMP 0x0 <MY_LABEL1-0x2a> \(BP\);
  72:	dc 13       	IF !CC JUMP 0x2a <MY_LABEL1>;
  74:	c6 13       	IF !CC JUMP 0x0 <MY_LABEL1-0x2a>;
  76:	da 17       	IF !CC JUMP 0x2a <MY_LABEL1> \(BP\);
  78:	c4 17       	IF !CC JUMP 0x0 <MY_LABEL1-0x2a> \(BP\);
  7a:	60 00       	CALL \(P0\);
  7c:	61 00       	CALL \(P1\);
  7e:	62 00       	CALL \(P2\);
  80:	63 00       	CALL \(P3\);
  82:	64 00       	CALL \(P4\);
  84:	65 00       	CALL \(P5\);
  86:	70 00       	CALL \(PC \+ P0\);
  88:	71 00       	CALL \(PC \+ P1\);
  8a:	72 00       	CALL \(PC \+ P2\);
  8c:	73 00       	CALL \(PC \+ P3\);
  8e:	74 00       	CALL \(PC \+ P4\);
  90:	75 00       	CALL \(PC \+ P5\);
  92:	09 e3 2b 1a 	CALL 0x1234e8 .*;
  96:	ff e3 97 fd 	CALL 0xfffffbc4 .*;
  9a:	ff e3 c8 ff 	CALL 0x2a <MY_LABEL1>;
  9e:	ff e3 b1 ff 	CALL 0x0 <MY_LABEL1-0x2a>;
  a2:	10 00       	RTS;
  a4:	11 00       	RTI;
  a6:	12 00       	RTX;
  a8:	13 00       	RTN;
  aa:	14 00       	RTE;
  ac:	82 e0 02 00 	LSETUP\(0xb0 <MY_LABEL1\+0x86>, 0xb0 <MY_LABEL1\+0x86>\) LC0;
  b0:	84 e0 06 00 	LSETUP\(0xb8 <beg_poll_bit>, 0xbc <end_poll_bit>\) LC0;
  b4:	00 00       	NOP;
	...

000000b8 <beg_poll_bit>:
  b8:	80 e1 01 00 	R0 = 0x1 \(Z\);.*

000000bc <end_poll_bit>:
  bc:	81 e1 02 00 	R1 = 0x2 \(Z\);.*
  c0:	92 e0 03 00 	LSETUP\(0xc4 <end_poll_bit\+0x8>, 0xc6 <end_poll_bit\+0xa>\) LC1;
  c4:	93 e0 05 00 	LSETUP\(0xca <FIR_filter>, 0xce <bottom_of_FIR_filter>\) LC1;
	...

000000ca <FIR_filter>:
  ca:	80 e1 01 00 	R0 = 0x1 \(Z\);.*

000000ce <bottom_of_FIR_filter>:
  ce:	81 e1 02 00 	R1 = 0x2 \(Z\);.*
  d2:	a2 e0 04 10 	LSETUP\(0xd6 <bottom_of_FIR_filter\+0x8>, 0xda <bottom_of_FIR_filter\+0xc>\) LC0 = P1;
  d6:	e2 e0 04 10 	LSETUP\(0xda <bottom_of_FIR_filter\+0xc>, 0xde <bottom_of_FIR_filter\+0x10>\) LC0 = P1 >> 0x1;
  da:	82 e0 03 00 	LSETUP\(0xde <bottom_of_FIR_filter\+0x10>, 0xe0 <bottom_of_FIR_filter\+0x12>\) LC0;
  de:	08 60       	R0 = 0x1 \(X\);.*
  e0:	11 60       	R1 = 0x2 \(X\);.*
  e2:	90 e0 00 00 	LSETUP\(0xe2 <bottom_of_FIR_filter\+0x14>, 0xe2 <bottom_of_FIR_filter\+0x14>\) LC1;
	...
