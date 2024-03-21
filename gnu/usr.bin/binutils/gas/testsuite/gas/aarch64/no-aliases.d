#source: alias.s
#objdump: -dr -Mno-aliases

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	13823c20 	extr	w0, w1, w2, #15
   4:	93c23c20 	extr	x0, x1, x2, #15
   8:	13831c60 	extr	w0, w3, w3, #7
   c:	93c51ca0 	extr	x0, x5, x5, #7
  10:	138748e6 	extr	w6, w7, w7, #18
  14:	93c7a0e6 	extr	x6, x7, x7, #40
  18:	1b020c20 	madd	w0, w1, w2, w3
  1c:	1b027c20 	madd	w0, w1, w2, wzr
  20:	1b027c20 	madd	w0, w1, w2, wzr
  24:	9b028c20 	msub	x0, x1, x2, x3
  28:	9b02fc20 	msub	x0, x1, x2, xzr
  2c:	9b02fc20 	msub	x0, x1, x2, xzr
  30:	9b220c20 	smaddl	x0, w1, w2, x3
  34:	9b227c20 	smaddl	x0, w1, w2, xzr
  38:	9b227c20 	smaddl	x0, w1, w2, xzr
  3c:	9b228c20 	smsubl	x0, w1, w2, x3
  40:	9b22fc20 	smsubl	x0, w1, w2, xzr
  44:	9b22fc20 	smsubl	x0, w1, w2, xzr
  48:	9ba20c20 	umaddl	x0, w1, w2, x3
  4c:	9ba27c20 	umaddl	x0, w1, w2, xzr
  50:	9ba27c20 	umaddl	x0, w1, w2, xzr
  54:	9ba28c20 	umsubl	x0, w1, w2, x3
  58:	9ba2fc20 	umsubl	x0, w1, w2, xzr
  5c:	9ba2fc20 	umsubl	x0, w1, w2, xzr
  60:	1a9f0420 	csinc	w0, w1, wzr, eq	// eq = none
  64:	1a810420 	csinc	w0, w1, w1, eq	// eq = none
  68:	1a810420 	csinc	w0, w1, w1, eq	// eq = none
  6c:	1a9f37e0 	csinc	w0, wzr, wzr, cc	// cc = lo, ul, last
  70:	1a9f37e0 	csinc	w0, wzr, wzr, cc	// cc = lo, ul, last
  74:	da9f2020 	csinv	x0, x1, xzr, cs	// cs = hs, nlast
  78:	da812020 	csinv	x0, x1, x1, cs	// cs = hs, nlast
  7c:	da812020 	csinv	x0, x1, x1, cs	// cs = hs, nlast
  80:	da9f43e0 	csinv	x0, xzr, xzr, mi	// mi = first
  84:	da9f43e0 	csinv	x0, xzr, xzr, mi	// mi = first
  88:	da9eb7e0 	csneg	x0, xzr, x30, lt	// lt = tstop
  8c:	da9eb7c0 	csneg	x0, x30, x30, lt	// lt = tstop
  90:	da9eb7c0 	csneg	x0, x30, x30, lt	// lt = tstop
  94:	ea020020 	ands	x0, x1, x2
  98:	ea02003f 	ands	xzr, x1, x2
  9c:	ea02003f 	ands	xzr, x1, x2
  a0:	6ac27c3f 	ands	wzr, w1, w2, ror #31
  a4:	6ac27c3f 	ands	wzr, w1, w2, ror #31
  a8:	aa220020 	orn	x0, x1, x2
  ac:	aa22003f 	orn	xzr, x1, x2
  b0:	aa2203e0 	orn	x0, xzr, x2
  b4:	aa2203e0 	orn	x0, xzr, x2
  b8:	2aa23c3f 	orn	wzr, w1, w2, asr #15
  bc:	2aa23fe0 	orn	w0, wzr, w2, asr #15
  c0:	2aa23fe0 	orn	w0, wzr, w2, asr #15
  c4:	0ea11c20 	orr	v0.8b, v1.8b, v1.8b
  c8:	0ea21c20 	orr	v0.8b, v1.8b, v2.8b
  cc:	0ea11c20 	orr	v0.8b, v1.8b, v1.8b
  d0:	aa1103e3 	orr	x3, xzr, x17
  d4:	aa110003 	orr	x3, x0, x17
  d8:	aa1103e3 	orr	x3, xzr, x17
  dc:	92628421 	and	x1, x1, #0xffffffffc0000000
  e0:	927ef800 	and	x0, x0, #0xfffffffffffffffd
  e4:	121e7800 	and	w0, w0, #0xfffffffd
  e8:	721d1f1f 	ands	wzr, w24, #0x7f8
  ec:	721d1f00 	ands	w0, w24, #0x7f8
  f0:	721d1f1f 	ands	wzr, w24, #0x7f8
  f4:	7100807f 	subs	wzr, w3, #0x20
  f8:	710083e3 	subs	w3, wsp, #0x20
  fc:	7100807f 	subs	wzr, w3, #0x20
 100:	b13ffdff 	adds	xzr, x15, #0xfff
 104:	f13fffef 	subs	x15, sp, #0xfff
 108:	b13ffdff 	adds	xzr, x15, #0xfff
 10c:	0f08a448 	sshll	v8.8h, v2.8b, #0
 110:	0f08a448 	sshll	v8.8h, v2.8b, #0
 114:	4f08a448 	sshll2	v8.8h, v2.16b, #0
 118:	4f08a448 	sshll2	v8.8h, v2.16b, #0
 11c:	0f10a448 	sshll	v8.4s, v2.4h, #0
 120:	0f10a448 	sshll	v8.4s, v2.4h, #0
 124:	4f10a448 	sshll2	v8.4s, v2.8h, #0
 128:	4f10a448 	sshll2	v8.4s, v2.8h, #0
 12c:	0f20a448 	sshll	v8.2d, v2.2s, #0
 130:	0f20a448 	sshll	v8.2d, v2.2s, #0
 134:	4f20a448 	sshll2	v8.2d, v2.4s, #0
 138:	4f20a448 	sshll2	v8.2d, v2.4s, #0
 13c:	2f08a448 	ushll	v8.8h, v2.8b, #0
 140:	2f08a448 	ushll	v8.8h, v2.8b, #0
 144:	6f08a448 	ushll2	v8.8h, v2.16b, #0
 148:	6f08a448 	ushll2	v8.8h, v2.16b, #0
 14c:	2f10a448 	ushll	v8.4s, v2.4h, #0
 150:	2f10a448 	ushll	v8.4s, v2.4h, #0
 154:	6f10a448 	ushll2	v8.4s, v2.8h, #0
 158:	6f10a448 	ushll2	v8.4s, v2.8h, #0
 15c:	2f20a448 	ushll	v8.2d, v2.2s, #0
 160:	2f20a448 	ushll	v8.2d, v2.2s, #0
 164:	6f20a448 	ushll2	v8.2d, v2.4s, #0
 168:	6f20a448 	ushll2	v8.2d, v2.4s, #0
 16c:	1a81f420 	csinc	w0, w1, w1, nv
 170:	1a81e420 	csinc	w0, w1, w1, al
 174:	1a9ff7e0 	csinc	w0, wzr, wzr, nv
 178:	1a9fe7e0 	csinc	w0, wzr, wzr, al
 17c:	5a81f020 	csinv	w0, w1, w1, nv
 180:	5a81e020 	csinv	w0, w1, w1, al
 184:	5a9ff3e0 	csinv	w0, wzr, wzr, nv
 188:	5a9fe3e0 	csinv	w0, wzr, wzr, al
 18c:	5a81f420 	csneg	w0, w1, w1, nv
 190:	5a81e420 	csneg	w0, w1, w1, al
