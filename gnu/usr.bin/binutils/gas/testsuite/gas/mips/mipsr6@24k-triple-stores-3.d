#objdump: -dr
#as: -mfix-24k -32
#name: 24K: Triple Store (Double-word Check)
#source: 24k-triple-stores-3.s

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
	\.\.\.
