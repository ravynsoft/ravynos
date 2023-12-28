#objdump: -dr 
#as: -mfix-24k -32
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
  54:	bba20000 	swr	v0,0\(sp\)
  58:	bba30008 	swr	v1,8\(sp\)
  5c:	00000000 	nop
  60:	bba40010 	swr	a0,16\(sp\)
  64:	bba50018 	swr	a1,24\(sp\)
  68:	00000000 	nop
  6c:	bba60020 	swr	a2,32\(sp\)
  70:	aba20000 	swl	v0,0\(sp\)
  74:	aba30008 	swl	v1,8\(sp\)
  78:	00000000 	nop
  7c:	aba40010 	swl	a0,16\(sp\)
  80:	aba50018 	swl	a1,24\(sp\)
  84:	00000000 	nop
  88:	aba60020 	swl	a2,32\(sp\)
  8c:	e3a20000 	sc	v0,0\(sp\)
  90:	e3a30008 	sc	v1,8\(sp\)
  94:	00000000 	nop
  98:	e3a40010 	sc	a0,16\(sp\)
  9c:	e3a50018 	sc	a1,24\(sp\)
  a0:	00000000 	nop
  a4:	e3a60020 	sc	a2,32\(sp\)
  a8:	e7a20000 	swc1	\$f2,0\(sp\)
  ac:	e7a30008 	swc1	\$f3,8\(sp\)
  b0:	00000000 	nop
  b4:	e7a40010 	swc1	\$f4,16\(sp\)
  b8:	e7a50018 	swc1	\$f5,24\(sp\)
  bc:	00000000 	nop
  c0:	e7a60020 	swc1	\$f6,32\(sp\)
  c4:	eba20000 	swc2	\$2,0\(sp\)
  c8:	eba30008 	swc2	\$3,8\(sp\)
  cc:	00000000 	nop
  d0:	eba40010 	swc2	\$4,16\(sp\)
  d4:	eba50018 	swc2	\$5,24\(sp\)
  d8:	00000000 	nop
  dc:	eba60020 	swc2	\$6,32\(sp\)
  e0:	f7a20000 	sdc1	\$f2,0\(sp\)
  e4:	f7a30008 	sdc1	\$f3,8\(sp\)
  e8:	00000000 	nop
  ec:	f7a40010 	sdc1	\$f4,16\(sp\)
  f0:	f7a50018 	sdc1	\$f5,24\(sp\)
  f4:	00000000 	nop
  f8:	f7a60020 	sdc1	\$f6,32\(sp\)
  fc:	fba20000 	sdc2	\$2,0\(sp\)
 100:	fba30008 	sdc2	\$3,8\(sp\)
 104:	00000000 	nop
 108:	fba40010 	sdc2	\$4,16\(sp\)
 10c:	fba50018 	sdc2	\$5,24\(sp\)
 110:	00000000 	nop
 114:	fba60020 	sdc2	\$6,32\(sp\)
 118:	4d090008 	swxc1	\$f0,t1\(t0\)
 11c:	00000000 	nop
 120:	4d0a0808 	swxc1	\$f1,t2\(t0\)
 124:	4d0b1008 	swxc1	\$f2,t3\(t0\)
 128:	00000000 	nop
 12c:	4d0c1808 	swxc1	\$f3,t4\(t0\)
 130:	4d0d2008 	swxc1	\$f4,t5\(t0\)
 134:	00000000 	nop
 138:	4d090009 	sdxc1	\$f0,t1\(t0\)
 13c:	4d0a1009 	sdxc1	\$f2,t2\(t0\)
 140:	00000000 	nop
 144:	4d0b2009 	sdxc1	\$f4,t3\(t0\)
 148:	4d0c3009 	sdxc1	\$f6,t4\(t0\)
 14c:	00000000 	nop
 150:	4d0d4009 	sdxc1	\$f8,t5\(t0\)
 154:	4d09000d 	suxc1	\$f0,t1\(t0\)
 158:	00000000 	nop
 15c:	4d0a100d 	suxc1	\$f2,t2\(t0\)
 160:	4d0b200d 	suxc1	\$f4,t3\(t0\)
 164:	00000000 	nop
 168:	4d0c300d 	suxc1	\$f6,t4\(t0\)
 16c:	4d0d400d 	suxc1	\$f8,t5\(t0\)
	...
