#objdump: -dr
#name: move
.*: +file format .*

Disassembly of section .text:

00000000 <move_register>:
   0:	38 31       	R7 = A0.X;
   2:	fb 32       	FP = B3;
   4:	35 36       	L2 = R5;
   6:	b2 34       	M2 = I2;
   8:	d8 39       	A1.W = USP;
   a:	06 31       	R0 = ASTAT;
   c:	c9 31       	R1 = SEQSTAT;
   e:	d2 31       	R2 = SYSCFG;
  10:	db 31       	R3 = RETI;
  12:	e4 31       	R4 = RETX;
  14:	ed 31       	R5 = RETN;
  16:	f6 31       	R6 = RETE;
  18:	3f 31       	R7 = RETS;
  1a:	a8 31       	R5 = LC0;
  1c:	a3 31       	R4 = LC1;
  1e:	99 31       	R3 = LT0;
  20:	94 31       	R2 = LT1;
  22:	8a 31       	R1 = LB0;
  24:	85 31       	R0 = LB1;
  26:	96 31       	R2 = CYCLES;
  28:	9f 31       	R3 = CYCLES2;
  2a:	cf 31       	R1 = EMUDAT;
  2c:	7f 38       	RETS = FP;
  2e:	e0 3d       	LT1 = USP;
  30:	72 38       	ASTAT = P2;
  32:	08 c4 [0|3][0|f] c0 	A0 = A1;
  36:	08 c4 [0|3][0|f] e0 	A1 = A0;
  3a:	09 c4 00 20 	A0 = R0;
  3e:	09 c4 08 a0 	A1 = R1;
  42:	8b c0 00 39 	R4 = A0 \(FU\);
  46:	2f c1 00 19 	R5 = A1 \(ISS2\);
  4a:	0b c0 80 39 	R6 = A0;
  4e:	0f c0 80 19 	R7 = A1;
  52:	0f c0 80 39 	R7 = A1, R6 = A0;
  56:	8f c0 00 38 	R1 = A1, R0 = A0 \(FU\);

0000005a <move_conditional>:
  5a:	6a 07       	IF CC R5 = P2;
  5c:	b0 06       	IF !CC SP = R0;

0000005e <move_half_to_full_zero_extend>:
  5e:	fa 42       	R2 = R7.L \(Z\);
  60:	c8 42       	R0 = R1.L \(Z\);

00000062 <move_half_to_full_sign_extend>:
  62:	8d 42       	R5 = R1.L \(X\);
  64:	93 42       	R3 = R2.L \(X\);

00000066 <move_register_half>:
  66:	09 c4 28 40 	A0.X = R5.L;
  6a:	09 c4 10 c0 	A1.X = R2.L;
  6e:	0a c4 3f 00 	R0.L = A0.X;
  72:	0a c4 3f 4e 	R7.L = A1.X;
  76:	09 c4 18 00 	A0.L = R3.L;
  7a:	09 c4 20 80 	A1.L = R4.L;
  7e:	29 c4 30 00 	A0.H = R6.H;
  82:	29 c4 28 80 	A1.H = R5.H;
  86:	83 c1 00 38 	R0.L = A0 \(IU\);
  8a:	27 c0 40 18 	R1.H = A1 \(S2RND\);
  8e:	07 c0 40 18 	R1.H = A1;
  92:	67 c1 80 38 	R2.H = A1, R2.L = A0 \(IH\);
  96:	07 c0 80 38 	R2.H = A1, R2.L = A0;
  9a:	47 c0 00 38 	R0.H = A1, R0.L = A0 \(T\);
  9e:	87 c0 00 38 	R0.H = A1, R0.L = A0 \(FU\);
  a2:	07 c1 00 38 	R0.H = A1, R0.L = A0 \(IS\);
  a6:	07 c0 00 38 	R0.H = A1, R0.L = A0;

000000aa <move_byte_zero_extend>:
  aa:	57 43       	R7 = R2.B \(Z\);
  ac:	48 43       	R0 = R1.B \(Z\);

000000ae <move_byte_sign_extend>:
  ae:	4e 43       	R6 = R1.B \(Z\);
  b0:	65 43       	R5 = R4.B \(Z\);
	...
