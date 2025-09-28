#as:
#objdump: -dr

.*:     file format pe-aarch64-little


Disassembly of section .text:

0000000000000000 <.text>:
	...

0000000000000010 <foo>:
  10:	12345678 	and	w24, w19, #0xfffff003
  14:	12345678 	and	w24, w19, #0xfffff003
  18:	00000010 	udf	#16
			18: IMAGE_REL_ARM64_ADDR32	.text
  1c:	00000010 	udf	#16
			1c: IMAGE_REL_ARM64_ADDR32	.text
	...
			20: IMAGE_REL_ARM64_ADDR32	bar
			24: IMAGE_REL_ARM64_ADDR32	bar
  28:	00000011 	udf	#17
			28: IMAGE_REL_ARM64_ADDR32	.text
  2c:	00000011 	udf	#17
			2c: IMAGE_REL_ARM64_ADDR32	.text
  30:	00000001 	udf	#1
			30: IMAGE_REL_ARM64_ADDR32	bar
  34:	00000001 	udf	#1
			34: IMAGE_REL_ARM64_ADDR32	bar
  38:	0000000f 	udf	#15
			38: IMAGE_REL_ARM64_ADDR32	.text
  3c:	0000000f 	udf	#15
			3c: IMAGE_REL_ARM64_ADDR32	.text
  40:	ffffffff 	.inst	0xffffffff ; undefined
			40: IMAGE_REL_ARM64_ADDR32	bar
  44:	ffffffff 	.inst	0xffffffff ; undefined
			44: IMAGE_REL_ARM64_ADDR32	bar
  48:	9abcdef0 	.inst	0x9abcdef0 ; undefined
  4c:	12345678 	and	w24, w19, #0xfffff003
  50:	9abcdef0 	.inst	0x9abcdef0 ; undefined
  54:	12345678 	and	w24, w19, #0xfffff003
  58:	00000010 	udf	#16
			58: IMAGE_REL_ARM64_ADDR64	.text
  5c:	00000000 	udf	#0
  60:	00000010 	udf	#16
			60: IMAGE_REL_ARM64_ADDR64	.text
	...
			68: IMAGE_REL_ARM64_ADDR64	bar
			70: IMAGE_REL_ARM64_ADDR64	bar
  78:	00000011 	udf	#17
			78: IMAGE_REL_ARM64_ADDR64	.text
  7c:	00000000 	udf	#0
  80:	00000011 	udf	#17
			80: IMAGE_REL_ARM64_ADDR64	.text
  84:	00000000 	udf	#0
  88:	00000001 	udf	#1
			88: IMAGE_REL_ARM64_ADDR64	bar
  8c:	00000000 	udf	#0
  90:	00000001 	udf	#1
			90: IMAGE_REL_ARM64_ADDR64	bar
  94:	00000000 	udf	#0
  98:	0000000f 	udf	#15
			98: IMAGE_REL_ARM64_ADDR64	.text
  9c:	00000000 	udf	#0
  a0:	0000000f 	udf	#15
			a0: IMAGE_REL_ARM64_ADDR64	.text
  a4:	00000000 	udf	#0
  a8:	ffffffff 	.inst	0xffffffff ; undefined
			a8: IMAGE_REL_ARM64_ADDR64	bar
  ac:	ffffffff 	.inst	0xffffffff ; undefined
  b0:	ffffffff 	.inst	0xffffffff ; undefined
			b0: IMAGE_REL_ARM64_ADDR64	bar
  b4:	ffffffff 	.inst	0xffffffff ; undefined
  b8:	00000010 	udf	#16
			b8: IMAGE_REL_ARM64_ADDR32NB	.text
  bc:	00000000 	udf	#0
			bc: IMAGE_REL_ARM64_ADDR32NB	bar
  c0:	00000011 	udf	#17
			c0: IMAGE_REL_ARM64_ADDR32NB	.text
  c4:	00000001 	udf	#1
			c4: IMAGE_REL_ARM64_ADDR32NB	bar
  c8:	0000000f 	udf	#15
			c8: IMAGE_REL_ARM64_ADDR32NB	.text
  cc:	ffffffff 	.inst	0xffffffff ; undefined
			cc: IMAGE_REL_ARM64_ADDR32NB	bar
  d0:	17ffffd0 	b	10 <foo>
  d4:	17ffffd0 	b	14 <foo\+0x4>
  d8:	17ffffcd 	b	c <.text\+0xc>
  dc:	14000000 	b	0 <bar>
			dc: IMAGE_REL_ARM64_BRANCH26	bar
  e0:	14000001 	b	4 <bar\+0x4>
			e0: IMAGE_REL_ARM64_BRANCH26	bar
  e4:	17ffffff 	b	fffffffffffffffc <bar\+0xfffffffffffffffc>
			e4: IMAGE_REL_ARM64_BRANCH26	bar
  e8:	97ffffca 	bl	10 <foo>
  ec:	97ffffca 	bl	14 <foo\+0x4>
  f0:	97ffffc7 	bl	c <.text\+0xc>
  f4:	94000000 	bl	0 <bar>
			f4: IMAGE_REL_ARM64_BRANCH26	bar
  f8:	94000001 	bl	4 <bar\+0x4>
			f8: IMAGE_REL_ARM64_BRANCH26	bar
  fc:	97ffffff 	bl	fffffffffffffffc <bar\+0xfffffffffffffffc>
			fc: IMAGE_REL_ARM64_BRANCH26	bar
 100:	97ffffbf 	bl	fffffffffffffffc <foo\+0xffffffffffffffec>
 104:	b4fff860 	cbz	x0, 10 <foo>
 108:	b4fff860 	cbz	x0, 14 <foo\+0x4>
 10c:	b4fff800 	cbz	x0, c <.text\+0xc>
 110:	b4000000 	cbz	x0, 0 <bar>
			110: IMAGE_REL_ARM64_BRANCH19	bar
 114:	b4000020 	cbz	x0, 4 <bar\+0x4>
			114: IMAGE_REL_ARM64_BRANCH19	bar
 118:	b4ffffe0 	cbz	x0, fffffffffffffffc <bar\+0xfffffffffffffffc>
			118: IMAGE_REL_ARM64_BRANCH19	bar
 11c:	b4fff700 	cbz	x0, fffffffffffffffc <foo\+0xffffffffffffffec>
 120:	3607f780 	tbz	w0, #0, 10 <foo>
 124:	3607f780 	tbz	w0, #0, 14 <foo\+0x4>
 128:	3607f720 	tbz	w0, #0, c <.text\+0xc>
 12c:	36000000 	tbz	w0, #0, 0 <bar>
			12c: IMAGE_REL_ARM64_BRANCH14	bar
 130:	36000020 	tbz	w0, #0, 4 <bar\+0x4>
			130: IMAGE_REL_ARM64_BRANCH14	bar
 134:	3607ffe0 	tbz	w0, #0, fffffffffffffffc <bar\+0xfffffffffffffffc>
			134: IMAGE_REL_ARM64_BRANCH14	bar
 138:	3607f620 	tbz	w0, #0, fffffffffffffffc <foo\+0xffffffffffffffec>
 13c:	90000080 	adrp	x0, 10000 <foo\+0xfff0>
			13c: IMAGE_REL_ARM64_PAGEBASE_REL21	.text
 140:	b0000080 	adrp	x0, 11000 <foo\+0x10ff0>
			140: IMAGE_REL_ARM64_PAGEBASE_REL21	.text
 144:	f0000060 	adrp	x0, f000 <foo\+0xeff0>
			144: IMAGE_REL_ARM64_PAGEBASE_REL21	.text
 148:	90000000 	adrp	x0, 0 <bar>
			148: IMAGE_REL_ARM64_PAGEBASE_REL21	bar
 14c:	b0000000 	adrp	x0, 1000 <bar\+0x1000>
			14c: IMAGE_REL_ARM64_PAGEBASE_REL21	bar
 150:	f0ffffe0 	adrp	x0, fffffffffffff000 <bar\+0xfffffffffffff000>
			150: IMAGE_REL_ARM64_PAGEBASE_REL21	bar
 154:	90ffffe0 	adrp	x0, ffffffffffffc000 <foo\+0xffffffffffffbff0>
			154: IMAGE_REL_ARM64_PAGEBASE_REL21	.text
 158:	10fff5c0 	adr	x0, 10 <foo>
 15c:	30fff5a0 	adr	x0, 11 <foo\+0x1>
 160:	70fff560 	adr	x0, f <.text\+0xf>
 164:	10000000 	adr	x0, 0 <bar>
			164: IMAGE_REL_ARM64_REL21	bar
 168:	30000000 	adr	x0, 1 <bar\+0x1>
			168: IMAGE_REL_ARM64_REL21	bar
 16c:	70ffffe0 	adr	x0, ffffffffffffffff <bar\+0xffffffffffffffff>
			16c: IMAGE_REL_ARM64_REL21	bar
 170:	70fff460 	adr	x0, ffffffffffffffff <foo\+0xffffffffffffffef>
 174:	39004000 	strb	w0, \[x0, #16\]
			174: IMAGE_REL_ARM64_PAGEOFFSET_12L	.text
 178:	39005000 	strb	w0, \[x0, #20\]
			178: IMAGE_REL_ARM64_PAGEOFFSET_12L	.text
 17c:	39003000 	strb	w0, \[x0, #12\]
			17c: IMAGE_REL_ARM64_PAGEOFFSET_12L	.text
 180:	39000000 	strb	w0, \[x0\]
			180: IMAGE_REL_ARM64_PAGEOFFSET_12L	bar
 184:	39001000 	strb	w0, \[x0, #4\]
			184: IMAGE_REL_ARM64_PAGEOFFSET_12L	bar
 188:	393ff000 	strb	w0, \[x0, #4092\]
			188: IMAGE_REL_ARM64_PAGEOFFSET_12L	bar
 18c:	393ff000 	strb	w0, \[x0, #4092\]
			18c: IMAGE_REL_ARM64_PAGEOFFSET_12L	.text
 190:	79002000 	strh	w0, \[x0, #16\]
			190: IMAGE_REL_ARM64_PAGEOFFSET_12L	.text
 194:	79002800 	strh	w0, \[x0, #20\]
			194: IMAGE_REL_ARM64_PAGEOFFSET_12L	.text
 198:	79001800 	strh	w0, \[x0, #12\]
			198: IMAGE_REL_ARM64_PAGEOFFSET_12L	.text
 19c:	79000000 	strh	w0, \[x0\]
			19c: IMAGE_REL_ARM64_PAGEOFFSET_12L	bar
 1a0:	79000800 	strh	w0, \[x0, #4\]
			1a0: IMAGE_REL_ARM64_PAGEOFFSET_12L	bar
 1a4:	791ff800 	strh	w0, \[x0, #4092\]
			1a4: IMAGE_REL_ARM64_PAGEOFFSET_12L	bar
 1a8:	791ff800 	strh	w0, \[x0, #4092\]
			1a8: IMAGE_REL_ARM64_PAGEOFFSET_12L	.text
 1ac:	b9001000 	str	w0, \[x0, #16\]
			1ac: IMAGE_REL_ARM64_PAGEOFFSET_12L	.text
 1b0:	b9001400 	str	w0, \[x0, #20\]
			1b0: IMAGE_REL_ARM64_PAGEOFFSET_12L	.text
 1b4:	b9000c00 	str	w0, \[x0, #12\]
			1b4: IMAGE_REL_ARM64_PAGEOFFSET_12L	.text
 1b8:	b9000000 	str	w0, \[x0\]
			1b8: IMAGE_REL_ARM64_PAGEOFFSET_12L	bar
 1bc:	b9000400 	str	w0, \[x0, #4\]
			1bc: IMAGE_REL_ARM64_PAGEOFFSET_12L	bar
 1c0:	b90ffc00 	str	w0, \[x0, #4092\]
			1c0: IMAGE_REL_ARM64_PAGEOFFSET_12L	bar
 1c4:	b90ffc00 	str	w0, \[x0, #4092\]
			1c4: IMAGE_REL_ARM64_PAGEOFFSET_12L	.text
 1c8:	f9000800 	str	x0, \[x0, #16\]
			1c8: IMAGE_REL_ARM64_PAGEOFFSET_12L	.text
 1cc:	f9000c00 	str	x0, \[x0, #24\]
			1cc: IMAGE_REL_ARM64_PAGEOFFSET_12L	.text
 1d0:	f9000400 	str	x0, \[x0, #8\]
			1d0: IMAGE_REL_ARM64_PAGEOFFSET_12L	.text
 1d4:	f9000000 	str	x0, \[x0\]
			1d4: IMAGE_REL_ARM64_PAGEOFFSET_12L	bar
 1d8:	f9000400 	str	x0, \[x0, #8\]
			1d8: IMAGE_REL_ARM64_PAGEOFFSET_12L	bar
 1dc:	f907fc00 	str	x0, \[x0, #4088\]
			1dc: IMAGE_REL_ARM64_PAGEOFFSET_12L	bar
 1e0:	f907fc00 	str	x0, \[x0, #4088\]
			1e0: IMAGE_REL_ARM64_PAGEOFFSET_12L	.text
 1e4:	3d800400 	str	q0, \[x0, #16\]
			1e4: IMAGE_REL_ARM64_PAGEOFFSET_12L	.text
 1e8:	3d800800 	str	q0, \[x0, #32\]
			1e8: IMAGE_REL_ARM64_PAGEOFFSET_12L	.text
 1ec:	3d800000 	str	q0, \[x0\]
			1ec: IMAGE_REL_ARM64_PAGEOFFSET_12L	.text
 1f0:	3d800000 	str	q0, \[x0\]
			1f0: IMAGE_REL_ARM64_PAGEOFFSET_12L	bar
 1f4:	3d800400 	str	q0, \[x0, #16\]
			1f4: IMAGE_REL_ARM64_PAGEOFFSET_12L	bar
 1f8:	3d83fc00 	str	q0, \[x0, #4080\]
			1f8: IMAGE_REL_ARM64_PAGEOFFSET_12L	bar
 1fc:	3d83fc00 	str	q0, \[x0, #4080\]
			1fc: IMAGE_REL_ARM64_PAGEOFFSET_12L	.text
 200:	91004000 	add	x0, x0, #0x10
			200: IMAGE_REL_ARM64_PAGEOFFSET_12A	.text
 204:	91004400 	add	x0, x0, #0x11
			204: IMAGE_REL_ARM64_PAGEOFFSET_12A	.text
 208:	91003c00 	add	x0, x0, #0xf
			208: IMAGE_REL_ARM64_PAGEOFFSET_12A	.text
 20c:	91000000 	add	x0, x0, #0x0
			20c: IMAGE_REL_ARM64_PAGEOFFSET_12A	bar
 210:	91000400 	add	x0, x0, #0x1
			210: IMAGE_REL_ARM64_PAGEOFFSET_12A	bar
 214:	913ffc00 	add	x0, x0, #0xfff
			214: IMAGE_REL_ARM64_PAGEOFFSET_12A	bar
 218:	913ffc00 	add	x0, x0, #0xfff
			218: IMAGE_REL_ARM64_PAGEOFFSET_12A	.text
