#source: ./bfmov.s
#objdump: -dr

.*:     file format .*


Disassembly of section .*:

00000000 <.*>:
   0:	fc 5e 00 00 04                	bfmov	#0, #0, #1, r0, r0
   5:	fc 5e 0f 00 04                	bfmov	#0, #0, #1, r0, r15
   a:	fc 5e f0 00 04                	bfmov	#0, #0, #1, r15, r0
   f:	fc 5e ff 00 04                	bfmov	#0, #0, #1, r15, r15
  14:	fc 5e 00 00 3c                	bfmov	#0, #0, #15, r0, r0
  19:	fc 5e 0f 00 3c                	bfmov	#0, #0, #15, r0, r15
  1e:	fc 5e f0 00 3c                	bfmov	#0, #0, #15, r15, r0
  23:	fc 5e ff 00 3c                	bfmov	#0, #0, #15, r15, r15
  28:	fc 5e 00 ef 41                	bfmov	#0, #15, #1, r0, r0
  2d:	fc 5e 0f ef 41                	bfmov	#0, #15, #1, r0, r15
  32:	fc 5e f0 ef 41                	bfmov	#0, #15, #1, r15, r0
  37:	fc 5e ff ef 41                	bfmov	#0, #15, #1, r15, r15
  3c:	fc 5e 00 ef 79                	bfmov	#0, #15, #15, r0, r0
  41:	fc 5e 0f ef 79                	bfmov	#0, #15, #15, r0, r15
  46:	fc 5e f0 ef 79                	bfmov	#0, #15, #15, r15, r0
  4b:	fc 5e ff ef 79                	bfmov	#0, #15, #15, r15, r15
  50:	fc 5e 00 11 04                	bfmov	#15, #0, #1, r0, r0
  55:	fc 5e 0f 11 04                	bfmov	#15, #0, #1, r0, r15
  5a:	fc 5e f0 11 04                	bfmov	#15, #0, #1, r15, r0
  5f:	fc 5e ff 11 04                	bfmov	#15, #0, #1, r15, r15
  64:	fc 5e 00 11 3c                	bfmov	#15, #0, #15, r0, r0
  69:	fc 5e 0f 11 3c                	bfmov	#15, #0, #15, r0, r15
  6e:	fc 5e f0 11 3c                	bfmov	#15, #0, #15, r15, r0
  73:	fc 5e ff 11 3c                	bfmov	#15, #0, #15, r15, r15
  78:	fc 5e 00 e0 41                	bfmov	#15, #15, #1, r0, r0
  7d:	fc 5e 0f e0 41                	bfmov	#15, #15, #1, r0, r15
  82:	fc 5e f0 e0 41                	bfmov	#15, #15, #1, r15, r0
  87:	fc 5e ff e0 41                	bfmov	#15, #15, #1, r15, r15
  8c:	fc 5e 00 e0 79                	bfmov	#15, #15, #15, r0, r0
  91:	fc 5e 0f e0 79                	bfmov	#15, #15, #15, r0, r15
  96:	fc 5e f0 e0 79                	bfmov	#15, #15, #15, r15, r0
  9b:	fc 5e ff e0 79                	bfmov	#15, #15, #15, r15, r15
  a0:	fc 5a 00 00 04                	bfmovz	#0, #0, #1, r0, r0
  a5:	fc 5a 0f 00 04                	bfmovz	#0, #0, #1, r0, r15
  aa:	fc 5a f0 00 04                	bfmovz	#0, #0, #1, r15, r0
  af:	fc 5a ff 00 04                	bfmovz	#0, #0, #1, r15, r15
  b4:	fc 5a 00 00 3c                	bfmovz	#0, #0, #15, r0, r0
  b9:	fc 5a 0f 00 3c                	bfmovz	#0, #0, #15, r0, r15
  be:	fc 5a f0 00 3c                	bfmovz	#0, #0, #15, r15, r0
  c3:	fc 5a ff 00 3c                	bfmovz	#0, #0, #15, r15, r15
  c8:	fc 5a 00 ef 41                	bfmovz	#0, #15, #1, r0, r0
  cd:	fc 5a 0f ef 41                	bfmovz	#0, #15, #1, r0, r15
  d2:	fc 5a f0 ef 41                	bfmovz	#0, #15, #1, r15, r0
  d7:	fc 5a ff ef 41                	bfmovz	#0, #15, #1, r15, r15
  dc:	fc 5a 00 ef 79                	bfmovz	#0, #15, #15, r0, r0
  e1:	fc 5a 0f ef 79                	bfmovz	#0, #15, #15, r0, r15
  e6:	fc 5a f0 ef 79                	bfmovz	#0, #15, #15, r15, r0
  eb:	fc 5a ff ef 79                	bfmovz	#0, #15, #15, r15, r15
  f0:	fc 5a 00 11 04                	bfmovz	#15, #0, #1, r0, r0
  f5:	fc 5a 0f 11 04                	bfmovz	#15, #0, #1, r0, r15
  fa:	fc 5a f0 11 04                	bfmovz	#15, #0, #1, r15, r0
  ff:	fc 5a ff 11 04                	bfmovz	#15, #0, #1, r15, r15
 104:	fc 5a 00 11 3c                	bfmovz	#15, #0, #15, r0, r0
 109:	fc 5a 0f 11 3c                	bfmovz	#15, #0, #15, r0, r15
 10e:	fc 5a f0 11 3c                	bfmovz	#15, #0, #15, r15, r0
 113:	fc 5a ff 11 3c                	bfmovz	#15, #0, #15, r15, r15
 118:	fc 5a 00 e0 41                	bfmovz	#15, #15, #1, r0, r0
 11d:	fc 5a 0f e0 41                	bfmovz	#15, #15, #1, r0, r15
 122:	fc 5a f0 e0 41                	bfmovz	#15, #15, #1, r15, r0
 127:	fc 5a ff e0 41                	bfmovz	#15, #15, #1, r15, r15
 12c:	fc 5a 00 e0 79                	bfmovz	#15, #15, #15, r0, r0
 131:	fc 5a 0f e0 79                	bfmovz	#15, #15, #15, r0, r15
 136:	fc 5a f0 e0 79                	bfmovz	#15, #15, #15, r15, r0
 13b:	fc 5a ff e0 79                	bfmovz	#15, #15, #15, r15, r15
