#objdump: -dr
#as: -mfix-24k -32
#name: 24K: Triple Store (Double-word Check)

.*: +file format .*mips.*

Disassembly of section .text:

0+ <.*>:
   0:	a3a2000b 	sb	v0,11\(sp\)
   4:	a3a3000b 	sb	v1,11\(sp\)
   8:	a3a40004 	sb	a0,4\(sp\)
   c:	0000000d 	break
  10:	a3a20000 	sb	v0,0\(sp\)
  14:	a3a3000b 	sb	v1,11\(sp\)
  18:	a3a40005 	sb	a0,5\(sp\)
  1c:	0000000d 	break
  20:	a3a20007 	sb	v0,7\(sp\)
  24:	a3a3000b 	sb	v1,11\(sp\)
  28:	00000000 	nop
  2c:	a3a40010 	sb	a0,16\(sp\)
  30:	0000000d 	break
  34:	a1020000 	sb	v0,0\(t0\)
  38:	a1030008 	sb	v1,8\(t0\)
  3c:	00000000 	nop
  40:	a1040009 	sb	a0,9\(t0\)
  44:	0000000d 	break
  48:	a7a20000 	sh	v0,0\(sp\)
  4c:	a7a3ffe1 	sh	v1,-31\(sp\)
  50:	a7a4ffe2 	sh	a0,-30\(sp\)
  54:	0000000d 	break
  58:	a7a20006 	sh	v0,6\(sp\)
  5c:	a7a30008 	sh	v1,8\(sp\)
  60:	00000000 	nop
  64:	a7a40010 	sh	a0,16\(sp\)
  68:	0000000d 	break
  6c:	a5020001 	sh	v0,1\(t0\)
  70:	a5030003 	sh	v1,3\(t0\)
  74:	00000000 	nop
  78:	a504000b 	sh	a0,11\(t0\)
  7c:	0000000d 	break
  80:	afa20008 	sw	v0,8\(sp\)
  84:	afa3fff8 	sw	v1,-8\(sp\)
  88:	afa40008 	sw	a0,8\(sp\)
  8c:	0000000d 	break
  90:	afa20004 	sw	v0,4\(sp\)
  94:	afa30008 	sw	v1,8\(sp\)
  98:	00000000 	nop
  9c:	afa40010 	sw	a0,16\(sp\)
  a0:	0000000d 	break
  a4:	ad020003 	sw	v0,3\(t0\)
  a8:	ad030007 	sw	v1,7\(t0\)
  ac:	00000000 	nop
  b0:	ad04000f 	sw	a0,15\(t0\)
  b4:	0000000d 	break
  b8:	aba20004 	swl	v0,4\(sp\)
  bc:	aba3000a 	swl	v1,10\(sp\)
  c0:	00000000 	nop
  c4:	aba40011 	swl	a0,17\(sp\)
  c8:	0000000d 	break
  cc:	aba20007 	swl	v0,7\(sp\)
  d0:	aba3000c 	swl	v1,12\(sp\)
  d4:	00000000 	nop
  d8:	aba40010 	swl	a0,16\(sp\)
  dc:	0000000d 	break
  e0:	aba20000 	swl	v0,0\(sp\)
  e4:	aba3000c 	swl	v1,12\(sp\)
  e8:	00000000 	nop
  ec:	aba40017 	swl	a0,23\(sp\)
  f0:	0000000d 	break
  f4:	a9020003 	swl	v0,3\(t0\)
  f8:	a9030008 	swl	v1,8\(t0\)
  fc:	00000000 	nop
 100:	a904000c 	swl	a0,12\(t0\)
 104:	0000000d 	break
 108:	aba20000 	swl	v0,0\(sp\)
 10c:	aba3000c 	swl	v1,12\(sp\)
 110:	00000000 	nop
 114:	bba40017 	swr	a0,23\(sp\)
 118:	0000000d 	break
 11c:	a9020005 	swl	v0,5\(t0\)
 120:	a9030011 	swl	v1,17\(t0\)
 124:	00000000 	nop
 128:	b904001c 	swr	a0,28\(t0\)
 12c:	0000000d 	break
	\.\.\.
