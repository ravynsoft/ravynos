#objdump: -dr
#name: arith_mode
.*: +file format .*


Disassembly of section .text:

00000000 <.text>:
   0:	03 c0 00 38 	R0.L = A0;
   4:	83 c0 00 38 	R0.L = A0 \(FU\);
   8:	03 c1 00 38 	R0.L = A0 \(IS\);
   c:	83 c1 00 38 	R0.L = A0 \(IU\);
  10:	43 c0 00 38 	R0.L = A0 \(T\);
  14:	c3 c0 00 38 	R0.L = A0 \(TFU\);
  18:	23 c0 00 38 	R0.L = A0 \(S2RND\);
  1c:	23 c1 00 38 	R0.L = A0 \(ISS2\);
  20:	63 c1 00 38 	R0.L = A0 \(IH\);
  24:	0b c0 00 38 	R0 = A0;
  28:	8b c0 00 38 	R0 = A0 \(FU\);
  2c:	0b c1 00 38 	R0 = A0 \(IS\);
  30:	8b c1 00 38 	R0 = A0 \(IU\);
  34:	2b c0 00 38 	R0 = A0 \(S2RND\);
  38:	2b c1 00 38 	R0 = A0 \(ISS2\);
  3c:	04 c2 0a 40 	R0.H = R1.L \* R2.H;
  40:	84 c2 0a 40 	R0.H = R1.L \* R2.H \(FU\);
  44:	04 c3 0a 40 	R0.H = R1.L \* R2.H \(IS\);
  48:	84 c3 0a 40 	R0.H = R1.L \* R2.H \(IU\);
  4c:	44 c2 0a 40 	R0.H = R1.L \* R2.H \(T\);
  50:	c4 c2 0a 40 	R0.H = R1.L \* R2.H \(TFU\);
  54:	24 c2 0a 40 	R0.H = R1.L \* R2.H \(S2RND\);
  58:	24 c3 0a 40 	R0.H = R1.L \* R2.H \(ISS2\);
  5c:	64 c3 0a 40 	R0.H = R1.L \* R2.H \(IH\);
  60:	08 c2 0a 22 	R0 = R1.L \* R2.H;
  64:	88 c2 0a 22 	R0 = R1.L \* R2.H \(FU\);
  68:	08 c3 0a 22 	R0 = R1.L \* R2.H \(IS\);
  6c:	28 c2 0a 22 	R0 = R1.L \* R2.H \(S2RND\);
  70:	28 c3 0a 22 	R0 = R1.L \* R2.H \(ISS2\);
  74:	03 c0 0a 02 	A0 = R1.L \* R2.H;
  78:	83 c0 0a 02 	A0 = R1.L \* R2.H \(FU\);
  7c:	03 c1 0a 02 	A0 = R1.L \* R2.H \(IS\);
  80:	63 c0 0a 02 	A0 = R1.L \* R2.H \(W32\);
  84:	03 c0 0a 22 	R0.L = \(A0 = R1.L \* R2.H\);
  88:	83 c0 0a 22 	R0.L = \(A0 = R1.L \* R2.H\) \(FU\);
  8c:	03 c1 0a 22 	R0.L = \(A0 = R1.L \* R2.H\) \(IS\);
  90:	83 c1 0a 22 	R0.L = \(A0 = R1.L \* R2.H\) \(IU\);
  94:	43 c0 0a 22 	R0.L = \(A0 = R1.L \* R2.H\) \(T\);
  98:	c3 c0 0a 22 	R0.L = \(A0 = R1.L \* R2.H\) \(TFU\);
  9c:	23 c0 0a 22 	R0.L = \(A0 = R1.L \* R2.H\) \(S2RND\);
  a0:	23 c1 0a 22 	R0.L = \(A0 = R1.L \* R2.H\) \(ISS2\);
  a4:	63 c1 0a 22 	R0.L = \(A0 = R1.L \* R2.H\) \(IH\);
  a8:	0b c0 0a 22 	R0 = \(A0 = R1.L \* R2.H\);
  ac:	8b c0 0a 22 	R0 = \(A0 = R1.L \* R2.H\) \(FU\);
  b0:	0b c1 0a 22 	R0 = \(A0 = R1.L \* R2.H\) \(IS\);
  b4:	8b c1 0a 22 	R0 = \(A0 = R1.L \* R2.H\) \(IU\);
  b8:	2b c0 0a 22 	R0 = \(A0 = R1.L \* R2.H\) \(S2RND\);
  bc:	2b c1 0a 22 	R0 = \(A0 = R1.L \* R2.H\) \(ISS2\);
