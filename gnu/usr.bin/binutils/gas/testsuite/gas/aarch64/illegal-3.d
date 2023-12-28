#name: Illegal Instructions - 3
#as:
#source: illegal-3.s
#objdump: -d

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	4dc2d4ec 	.inst	0x4dc2d4ec ; undefined
   4:	4de2d4fc 	.inst	0x4de2d4fc ; undefined
   8:	4dc2f4ec 	.inst	0x4dc2f4ec ; undefined
   c:	4de2f4fc 	.inst	0x4de2f4fc ; undefined
  10:	1ea04000 	.inst	0x1ea04000 ; undefined
  14:	1ea01000 	.inst	0x1ea01000 ; undefined
  18:	2f00f400 	.inst	0x2f00f400 ; undefined
  1c:	1ea60000 	.inst	0x1ea60000 ; undefined
  20:	1ea70000 	.inst	0x1ea70000 ; undefined
  24:	9ea60000 	.inst	0x9ea60000 ; undefined
  28:	9ea70000 	.inst	0x9ea70000 ; undefined
  2c:	9e260000 	.inst	0x9e260000 ; undefined
  30:	9e270000 	.inst	0x9e270000 ; undefined
  34:	1e660000 	.inst	0x1e660000 ; undefined
  38:	1e670000 	.inst	0x1e670000 ; undefined
  3c:	1e2e0000 	.inst	0x1e2e0000 ; undefined
  40:	1e2f0000 	.inst	0x1e2f0000 ; undefined
  44:	1e6e0000 	.inst	0x1e6e0000 ; undefined
  48:	1e6f0000 	.inst	0x1e6f0000 ; undefined
  4c:	1eae0000 	.inst	0x1eae0000 ; undefined
  50:	1eaf0000 	.inst	0x1eaf0000 ; undefined
  54:	1eee0000 	.inst	0x1eee0000 ; undefined
  58:	1eef0000 	.inst	0x1eef0000 ; undefined
  5c:	1e2e0000 	.inst	0x1e2e0000 ; undefined
  60:	1e2f0000 	.inst	0x1e2f0000 ; undefined
  64:	1e6e0000 	.inst	0x1e6e0000 ; undefined
  68:	1e6f0000 	.inst	0x1e6f0000 ; undefined
  6c:	1eee0000 	.inst	0x1eee0000 ; undefined
  70:	1eef0000 	.inst	0x1eef0000 ; undefined
  74:	9e2e0000 	.inst	0x9e2e0000 ; undefined
  78:	9e2f0000 	.inst	0x9e2f0000 ; undefined
  7c:	9e6e0000 	.inst	0x9e6e0000 ; undefined
  80:	9e6f0000 	.inst	0x9e6f0000 ; undefined
  84:	9eee0000 	.inst	0x9eee0000 ; undefined
  88:	9eef0000 	.inst	0x9eef0000 ; undefined
  8c:	1ea60000 	.inst	0x1ea60000 ; undefined
  90:	1ea70000 	.inst	0x1ea70000 ; undefined
  94:	9ea60000 	.inst	0x9ea60000 ; undefined
  98:	9ea70000 	.inst	0x9ea70000 ; undefined
