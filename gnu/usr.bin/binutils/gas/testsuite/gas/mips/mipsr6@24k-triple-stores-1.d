#objdump: -dr
#as: -mfix-24k -32
#source: 24k-triple-stores-1.s
#name: 24K: Triple Store (Opcode Check)

.*: +file format .*mips.*

Disassembly of section .text:
0+ <.*>:
   0:	a3a20000 	sb	v0,0\(sp\)
   4:	a3a30008 	sb	v1,8\(sp\)
   8:	00000000 	nop
   c:	a3a40010 	sb	a0,16\(sp\)
  10:	a3a50018 	sb	a1,24\(sp\)
  14:	00000000 	nop
  18:	a3a60020 	sb	a2,32\(sp\)
  1c:	a7a20000 	sh	v0,0\(sp\)
  20:	a7a30008 	sh	v1,8\(sp\)
  24:	00000000 	nop
  28:	a7a40010 	sh	a0,16\(sp\)
  2c:	a7a50018 	sh	a1,24\(sp\)
  30:	00000000 	nop
  34:	a7a60020 	sh	a2,32\(sp\)
  38:	afa20000 	sw	v0,0\(sp\)
  3c:	afa30008 	sw	v1,8\(sp\)
  40:	00000000 	nop
  44:	afa40010 	sw	a0,16\(sp\)
  48:	afa50018 	sw	a1,24\(sp\)
  4c:	00000000 	nop
  50:	afa60020 	sw	a2,32\(sp\)
  54:	7fa20026 	sc	v0,0\(sp\)
  58:	00000000 	nop
  5c:	7fa30426 	sc	v1,8\(sp\)
  60:	7fa40826 	sc	a0,16\(sp\)
  64:	00000000 	nop
  68:	7fa50c26 	sc	a1,24\(sp\)
  6c:	7fa61026 	sc	a2,32\(sp\)
  70:	00000000 	nop
  74:	e7a20000 	swc1	\$f2,0\(sp\)
  78:	e7a30008 	swc1	\$f3,8\(sp\)
  7c:	00000000 	nop
  80:	e7a40010 	swc1	\$f4,16\(sp\)
  84:	e7a50018 	swc1	\$f5,24\(sp\)
  88:	00000000 	nop
  8c:	e7a60020 	swc1	\$f6,32\(sp\)
  90:	4962e800 	swc2	\$2,0\(sp\)
  94:	00000000 	nop
  98:	4963e808 	swc2	\$3,8\(sp\)
  9c:	4964e810 	swc2	\$4,16\(sp\)
  a0:	00000000 	nop
  a4:	4965e818 	swc2	\$5,24\(sp\)
  a8:	4966e820 	swc2	\$6,32\(sp\)
  ac:	00000000 	nop
  b0:	f7a20000 	sdc1	\$f2,0\(sp\)
  b4:	f7a30008 	sdc1	\$f3,8\(sp\)
  b8:	00000000 	nop
  bc:	f7a40010 	sdc1	\$f4,16\(sp\)
  c0:	f7a50018 	sdc1	\$f5,24\(sp\)
  c4:	00000000 	nop
  c8:	f7a60020 	sdc1	\$f6,32\(sp\)
  cc:	49e2e800 	sdc2	\$2,0\(sp\)
  d0:	00000000 	nop
  d4:	49e3e808 	sdc2	\$3,8\(sp\)
  d8:	49e4e810 	sdc2	\$4,16\(sp\)
  dc:	00000000 	nop
  e0:	49e5e818 	sdc2	\$5,24\(sp\)
  e4:	49e6e820 	sdc2	\$6,32\(sp\)
	...
