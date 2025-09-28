#objdump: -d -Mppc64
#as: -mppc64
#name: PowerPC test 3, simplified shifts

.*

Disassembly of section .text:

0+ <.text>:
   0:	(e0 0f 64 78|78 64 0f e0) 	srdi    r4,r3,63
   4:	(0e f8 83 78|78 83 f8 0e) 	rldimi  r3,r4,63,0
   8:	(e4 45 a5 78|78 a5 45 e4) 	sldi    r5,r5,8
   c:	(20 00 64 78|78 64 00 20) 	clrldi  r4,r3,32
  10:	(fe 0f 64 54|54 64 0f fe) 	srwi    r4,r3,31
  14:	(00 f8 83 50|50 83 f8 00) 	rlwimi  r3,r4,31,0,0
  18:	(2e 40 a5 54|54 a5 40 2e) 	slwi    r5,r5,8
  1c:	(3e 04 64 54|54 64 04 3e) 	clrlwi  r4,r3,16
  20:	(04 00 64 78|78 64 00 04) 	clrrdi  r4,r3,63
  24:	(e4 07 64 78|78 64 07 e4) 	clrrdi  r4,r3,0
  28:	(06 f8 64 78|78 64 f8 06) 	sldi    r4,r3,63
  2c:	(e6 ff 64 78|78 64 ff e6) 	rldicr  r4,r3,63,63
  30:	(42 f8 64 78|78 64 f8 42) 	srdi    r4,r3,1
  34:	(e2 ff 64 78|78 64 ff e2) 	rldicl  r4,r3,63,63
  38:	(0c 00 64 78|78 64 00 0c) 	rldimi  r4,r3,0,0
  3c:	(0c 08 64 78|78 64 08 0c) 	rldimi  r4,r3,1,0
  40:	(ac 0f 64 78|78 64 0f ac) 	rldimi  r4,r3,1,62
  44:	(ec 07 64 78|78 64 07 ec) 	rldimi  r4,r3,0,63
  48:	(00 00 64 78|78 64 00 00) 	rotldi  r4,r3,0
  4c:	(00 08 64 78|78 64 08 00) 	rotldi  r4,r3,1
  50:	(02 f8 64 78|78 64 f8 02) 	rotldi  r4,r3,63
  54:	(00 00 64 78|78 64 00 00) 	rotldi  r4,r3,0
  58:	(02 f8 64 78|78 64 f8 02) 	rotldi  r4,r3,63
  5c:	(00 08 64 78|78 64 08 00) 	rotldi  r4,r3,1
  60:	(10 20 65 78|78 65 20 10) 	rotld   r5,r3,r4
  64:	(e4 07 64 78|78 64 07 e4) 	clrrdi  r4,r3,0
  68:	(06 f8 64 78|78 64 f8 06) 	sldi    r4,r3,63
  6c:	(00 00 64 78|78 64 00 00) 	rotldi  r4,r3,0
  70:	(42 f8 64 78|78 64 f8 42) 	srdi    r4,r3,1
  74:	(e0 0f 64 78|78 64 0f e0) 	srdi    r4,r3,63
  78:	(00 00 64 78|78 64 00 00) 	rotldi  r4,r3,0
  7c:	(40 00 64 78|78 64 00 40) 	clrldi  r4,r3,1
  80:	(e0 07 64 78|78 64 07 e0) 	clrldi  r4,r3,63
  84:	(e4 07 64 78|78 64 07 e4) 	clrrdi  r4,r3,0
  88:	(a4 07 64 78|78 64 07 a4) 	clrrdi  r4,r3,1
  8c:	(04 00 64 78|78 64 00 04) 	clrrdi  r4,r3,63
  90:	(08 00 64 78|78 64 00 08) 	rldic   r4,r3,0,0
  94:	(48 00 64 78|78 64 00 48) 	rldic   r4,r3,0,1
  98:	(e8 07 64 78|78 64 07 e8) 	rldic   r4,r3,0,63
  9c:	(a8 0f 64 78|78 64 0f a8) 	rldic   r4,r3,1,62
  a0:	(0a f8 64 78|78 64 f8 0a) 	rldic   r4,r3,63,0
  a4:	(00 00 64 54|54 64 00 00) 	clrrwi  r4,r3,31
  a8:	(3e 00 64 54|54 64 00 3e) 	rotlwi  r4,r3,0
  ac:	(00 f8 64 54|54 64 f8 00) 	slwi    r4,r3,31
  b0:	(3e f8 64 54|54 64 f8 3e) 	rotlwi  r4,r3,31
  b4:	(7e f8 64 54|54 64 f8 7e) 	srwi    r4,r3,1
  b8:	(fe ff 64 54|54 64 ff fe) 	rlwinm  r4,r3,31,31,31
  bc:	(00 00 64 50|50 64 00 00) 	rlwimi  r4,r3,0,0,0
  c0:	(3e 00 64 50|50 64 00 3e) 	rlwimi  r4,r3,0,0,31
  c4:	(fe 0f 64 50|50 64 0f fe) 	rlwimi  r4,r3,1,31,31
  c8:	(00 f8 64 50|50 64 f8 00) 	rlwimi  r4,r3,31,0,0
  cc:	(3e 00 64 50|50 64 00 3e) 	rlwimi  r4,r3,0,0,31
  d0:	(fe 07 64 50|50 64 07 fe) 	rlwimi  r4,r3,0,31,31
  d4:	(3e 00 64 54|54 64 00 3e) 	rotlwi  r4,r3,0
  d8:	(3e 08 64 54|54 64 08 3e) 	rotlwi  r4,r3,1
  dc:	(3e f8 64 54|54 64 f8 3e) 	rotlwi  r4,r3,31
  e0:	(3e 00 64 54|54 64 00 3e) 	rotlwi  r4,r3,0
  e4:	(3e f8 64 54|54 64 f8 3e) 	rotlwi  r4,r3,31
  e8:	(3e 08 64 54|54 64 08 3e) 	rotlwi  r4,r3,1
  ec:	(3e 20 65 5c|5c 65 20 3e) 	rotlw   r5,r3,r4
  f0:	(3e 00 64 54|54 64 00 3e) 	rotlwi  r4,r3,0
  f4:	(3c 08 64 54|54 64 08 3c) 	slwi    r4,r3,1
  f8:	(00 f8 64 54|54 64 f8 00) 	slwi    r4,r3,31
  fc:	(3e 00 64 54|54 64 00 3e) 	rotlwi  r4,r3,0
 100:	(7e f8 64 54|54 64 f8 7e) 	srwi    r4,r3,1
 104:	(fe 0f 64 54|54 64 0f fe) 	srwi    r4,r3,31
 108:	(3e 00 64 54|54 64 00 3e) 	rotlwi  r4,r3,0
 10c:	(7e 00 64 54|54 64 00 7e) 	clrlwi  r4,r3,1
 110:	(fe 07 64 54|54 64 07 fe) 	clrlwi  r4,r3,31
 114:	(3e 00 64 54|54 64 00 3e) 	rotlwi  r4,r3,0
 118:	(3c 00 64 54|54 64 00 3c) 	clrrwi  r4,r3,1
 11c:	(00 00 64 54|54 64 00 00) 	clrrwi  r4,r3,31
 120:	(3e 00 64 54|54 64 00 3e) 	rotlwi  r4,r3,0
 124:	(7e 00 64 54|54 64 00 7e) 	clrlwi  r4,r3,1
 128:	(fe 07 64 54|54 64 07 fe) 	clrlwi  r4,r3,31
 12c:	(bc 0f 64 54|54 64 0f bc) 	rlwinm  r4,r3,1,30,30
 130:	(00 f8 64 54|54 64 f8 00) 	slwi    r4,r3,31
#pass
