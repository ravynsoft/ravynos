#objdump: -dr
#name: pseudo
.*: +file format .*
Disassembly of section .text:

00000000 <debug>:
   0:	00 f8       	DBG R0;
   2:	01 f8       	DBG R1;
   4:	02 f8       	DBG R2;
   6:	03 f8       	DBG R3;
   8:	04 f8       	DBG R4;
   a:	05 f8       	DBG R5;
   c:	06 f8       	DBG R6;
   e:	07 f8       	DBG R7;
  10:	08 f8       	DBG P0;
  12:	09 f8       	DBG P1;
  14:	0a f8       	DBG P2;
  16:	0b f8       	DBG P3;
  18:	0c f8       	DBG P4;
  1a:	0d f8       	DBG P5;
  1c:	0e f8       	DBG SP;
  1e:	0f f8       	DBG FP;
  20:	10 f8       	DBG I0;
  22:	11 f8       	DBG I1;
  24:	12 f8       	DBG I2;
  26:	13 f8       	DBG I3;
  28:	14 f8       	DBG M0;
  2a:	15 f8       	DBG M1;
  2c:	16 f8       	DBG M2;
  2e:	17 f8       	DBG M3;
  30:	18 f8       	DBG B0;
  32:	19 f8       	DBG B1;
  34:	1a f8       	DBG B2;
  36:	1b f8       	DBG B3;
  38:	1c f8       	DBG L0;
  3a:	1d f8       	DBG L1;
  3c:	1e f8       	DBG L2;
  3e:	1f f8       	DBG L3;
  40:	20 f8       	DBG A0.X;
  42:	21 f8       	DBG A0.W;
  44:	22 f8       	DBG A1.X;
  46:	23 f8       	DBG A1.W;
  48:	26 f8       	DBG ASTAT;
  4a:	27 f8       	DBG RETS;
  4c:	30 f8       	DBG LC0;
  4e:	31 f8       	DBG LT0;
  50:	32 f8       	DBG LB0;
  52:	33 f8       	DBG LC1;
  54:	34 f8       	DBG LT1;
  56:	35 f8       	DBG LB1;
  58:	36 f8       	DBG CYCLES;
  5a:	37 f8       	DBG CYCLES2;
  5c:	38 f8       	DBG USP;
  5e:	39 f8       	DBG SEQSTAT;
  60:	3a f8       	DBG SYSCFG;
  62:	3b f8       	DBG RETI;
  64:	3c f8       	DBG RETX;
  66:	3d f8       	DBG RETN;
  68:	3e f8       	DBG RETE;
  6a:	3f f8       	DBG EMUDAT;

0000006c <debug_assert>:
  6c:	00 f0 00 00 	DBGA \(R0.L, 0x0\);
  70:	40 f0 10 00 	DBGA \(R0.H, 0x10\);
  74:	00 f0 00 02 	DBGA \(R0.L, 0x200\);
  78:	40 f0 00 30 	DBGA \(R0.H, 0x3000\);
  7c:	01 f0 01 00 	DBGA \(R1.L, 0x1\);
  80:	41 f0 01 10 	DBGA \(R1.H, 0x1001\);
  84:	01 f0 08 80 	DBGA \(R1.L, 0x8008\);
  88:	41 f0 00 c0 	DBGA \(R1.H, 0xc000\);
  8c:	02 f0 00 04 	DBGA \(R2.L, 0x400\);
  90:	42 f0 00 08 	DBGA \(R2.H, 0x800\);
  94:	02 f0 00 10 	DBGA \(R2.L, 0x1000\);
  98:	42 f0 00 20 	DBGA \(R2.H, 0x2000\);
  9c:	03 f0 ff ff 	DBGA \(R3.L, 0xffff\);
  a0:	43 f0 ff 7f 	DBGA \(R3.H, 0x7fff\);
  a4:	03 f0 ff 3f 	DBGA \(R3.L, 0x3fff\);
  a8:	43 f0 ff 1f 	DBGA \(R3.H, 0x1fff\);
  ac:	0b f0 ff ff 	DBGA \(P3.L, 0xffff\);
  b0:	4b f0 9c ff 	DBGA \(P3.H, 0xff9c\);
  b4:	0b f0 18 fc 	DBGA \(P3.L, 0xfc18\);
  b8:	4b f0 01 e0 	DBGA \(P3.H, 0xe001\);
