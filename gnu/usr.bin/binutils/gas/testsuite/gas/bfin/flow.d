#objdump: -d
#name: flow
.*: +file format .*

Disassembly of section .text:

00000000 <jump>:
   0:	55 00       	JUMP \(P5\);
   2:	83 00       	JUMP \(PC \+ P3\);
   4:	00 20       	JUMP.S 0x4 .*
   6:	80 e2 00 00 	JUMP.L 0xff000006 .*
   a:	7f e2 ff ff 	JUMP.L 0x1000008 .*
   e:	ff 27       	JUMP.S 0x100c .*
  10:	7f e2 00 80 	JUMP.L 0xff0010 .*
  14:	f6 2f       	JUMP.S 0x0 <jump>;

00000016 <ccjump>:
  16:	00 1a       	IF CC JUMP 0xfffffc16 .*
  18:	ff 1d       	IF CC JUMP 0x416 .* \(BP\);
  1a:	00 16       	IF !CC JUMP 0xfffffc1a .* \(BP\);
  1c:	89 10       	IF !CC JUMP 0x12e .*
  1e:	f1 1b       	IF CC JUMP 0x0 <jump>;
  20:	f0 1f       	IF CC JUMP 0x0 <jump> \(BP\);
  22:	ef 17       	IF !CC JUMP 0x0 <jump> \(BP\);
  24:	ee 13       	IF !CC JUMP 0x0 <jump>;

00000026 <call>:
  26:	63 00       	CALL \(P3\);
  28:	72 00       	CALL \(PC \+ P2\);
  2a:	80 e3 00 00 	CALL 0xff00002a .*
  2e:	7f e3 ff ff 	CALL 0x100002c .*
  32:	ff e3 e7 ff 	CALL 0x0 <jump>;

00000036 <return>:
  36:	10 00       	RTS;
  38:	11 00       	RTI;
  3a:	12 00       	RTX;
  3c:	13 00       	RTN;
  3e:	14 00       	RTE;

00000040 <loop_lc0>:
  40:	82 e0 13 00 	LSETUP\(0x44 <loop_lc0\+0x4>, 0x66 <loop_lc0\+0x26>\) LC0;
  44:	38 e4 7b fc 	R0 = \[FP \+ -0xe14\];
  48:	49 60       	R1 = 0x9 \(X\);.*
  4a:	38 e4 7b fc 	R0 = \[FP \+ -0xe14\];
  4e:	00 32       	P0 = R0;
  50:	42 44       	P2 = P0 << 0x2;
  52:	ba 5a       	P2 = P2 \+ FP;
  54:	20 e1 50 fb 	R0 = -0x4b0 \(X\);.*
  58:	08 32       	P1 = R0;
  5a:	8a 5a       	P2 = P2 \+ P1;
  5c:	00 60       	R0 = 0x0 \(X\);.*
  5e:	10 93       	\[P2\] = R0;
  60:	38 e4 7b fc 	R0 = \[FP \+ -0xe14\];
  64:	08 64       	R0 \+= 0x1;.*
  66:	38 e6 7b fc 	\[FP \+ -0xe14\] = R0;
  6a:	a2 e0 02 40 	LSETUP\(0x6e <loop_lc0\+0x2e>, 0x6e <loop_lc0\+0x2e>\) LC0 = P4;
  6e:	00 00       	NOP;
  70:	e0 e0 00 10 	LSETUP\(0x70 <loop_lc0\+0x30>, 0x70 <loop_lc0\+0x30>\) LC0 = P1 >> 0x1;
  74:	82 e0 ff 03 	LSETUP\(0x78 <loop_lc0\+0x38>, 0x72 <loop_lc0\+0x32>\) LC0;
  78:	af e0 00 52 	LSETUP\(0x76 <loop_lc0\+0x36>, 0xfffffc78 <loop_lc1\+0xfffffbf8>\) LC0 = P5;
  7c:	ef e0 02 00 	LSETUP\(0x7a <loop_lc0\+0x3a>, 0x80 <loop_lc1>\) LC0 = P0 >> 0x1;

00000080 <loop_lc1>:
  80:	90 e0 00 00 	LSETUP\(0x80 <loop_lc1>, 0x80 <loop_lc1>\) LC1;
  84:	b0 e0 00 40 	LSETUP\(0x84 <loop_lc1\+0x4>, 0x84 <loop_lc1\+0x4>\) LC1 = P4;
  88:	f8 e0 1b 10 	LSETUP\(0x78 <loop_lc0\+0x38>, 0xbe <loop_lc1\+0x3e>\) LC1 = P1 >> 0x1;
  8c:	92 e0 ff 03 	LSETUP\(0x90 <loop_lc1\+0x10>, 0x8a <loop_lc1\+0xa>\) LC1;
  90:	bf e0 00 52 	LSETUP\(0x8e <loop_lc1\+0xe>, 0xfffffc90 <loop_lc1\+0xfffffc10>\) LC1 = P5;
  94:	ff e0 02 00 	LSETUP\(0x92 <loop_lc1\+0x12>, 0x98 <loop_lc1\+0x18>\) LC1 = P0 >> 0x1;
  98:	38 e4 7a fc 	R0 = \[FP \+ -0xe18\];
  9c:	00 32       	P0 = R0;
  9e:	42 44       	P2 = P0 << 0x2;
  a0:	ba 5a       	P2 = P2 \+ FP;
  a2:	20 e1 f0 f1 	R0 = -0xe10 \(X\);.*
  a6:	00 32       	P0 = R0;
  a8:	42 5a       	P1 = P2 \+ P0;
  aa:	38 e4 7a fc 	R0 = \[FP \+ -0xe18\];
  ae:	00 32       	P0 = R0;
  b0:	42 44       	P2 = P0 << 0x2;
  b2:	ba 5a       	P2 = P2 \+ FP;
  b4:	20 e1 50 fb 	R0 = -0x4b0 \(X\);.*
  b8:	00 32       	P0 = R0;
  ba:	82 5a       	P2 = P2 \+ P0;
  bc:	10 91       	R0 = \[P2\];
  be:	08 93       	\[P1\] = R0;
