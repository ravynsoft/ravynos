
tmpdir/aarch64.x:     file format pei-aarch64-little


Disassembly of section .text:

0000000000002000 <__rt_psrelocs_end>:
	...

0000000000002010 <foo>:
    2010:	12345678 	and	w24, w19, #0xfffff003
    2014:	12345678 	and	w24, w19, #0xfffff003
    2018:	00002010 	udf	#8208
    201c:	00002010 	udf	#8208
    2020:	00002220 	udf	#8736
    2024:	00002220 	udf	#8736
    2028:	00002011 	udf	#8209
    202c:	00002011 	udf	#8209
    2030:	00002221 	udf	#8737
    2034:	00002221 	udf	#8737
    2038:	0000200f 	udf	#8207
    203c:	0000200f 	udf	#8207
    2040:	0000221f 	udf	#8735
    2044:	0000221f 	udf	#8735
    2048:	9abcdef0 	.inst	0x9abcdef0 ; undefined
    204c:	12345678 	and	w24, w19, #0xfffff003
    2050:	9abcdef0 	.inst	0x9abcdef0 ; undefined
    2054:	12345678 	and	w24, w19, #0xfffff003
    2058:	00002010 	udf	#8208
    205c:	00000000 	udf	#0
    2060:	00002010 	udf	#8208
    2064:	00000000 	udf	#0
    2068:	00002220 	udf	#8736
    206c:	00000000 	udf	#0
    2070:	00002220 	udf	#8736
    2074:	00000000 	udf	#0
    2078:	00002011 	udf	#8209
    207c:	00000000 	udf	#0
    2080:	00002011 	udf	#8209
    2084:	00000000 	udf	#0
    2088:	00002221 	udf	#8737
    208c:	00000000 	udf	#0
    2090:	00002221 	udf	#8737
    2094:	00000000 	udf	#0
    2098:	0000200f 	udf	#8207
    209c:	00000000 	udf	#0
    20a0:	0000200f 	udf	#8207
    20a4:	00000000 	udf	#0
    20a8:	0000221f 	udf	#8735
    20ac:	00000000 	udf	#0
    20b0:	0000221f 	udf	#8735
    20b4:	00000000 	udf	#0
    20b8:	00001010 	udf	#4112
    20bc:	00001220 	udf	#4640
    20c0:	00001011 	udf	#4113
    20c4:	00001221 	udf	#4641
    20c8:	0000100f 	udf	#4111
    20cc:	0000121f 	udf	#4639
    20d0:	17ffffd0 	b	2010 <foo>
    20d4:	17ffffd0 	b	2014 <foo\+0x4>
    20d8:	17ffffcd 	b	200c <__rt_psrelocs_end\+0xc>
    20dc:	14000051 	b	2220 <bar>
    20e0:	14000051 	b	2224 <bar\+0x4>
    20e4:	1400004e 	b	221c <.text>
    20e8:	97ffffca 	bl	2010 <foo>
    20ec:	97ffffca 	bl	2014 <foo\+0x4>
    20f0:	97ffffc7 	bl	200c <__rt_psrelocs_end\+0xc>
    20f4:	9400004b 	bl	2220 <bar>
    20f8:	9400004b 	bl	2224 <bar\+0x4>
    20fc:	94000048 	bl	221c <.text>
    2100:	97ffffbf 	bl	1ffc <__ImageBase\+0xffc>
    2104:	b4fff860 	cbz	x0, 2010 <foo>
    2108:	b4fff860 	cbz	x0, 2014 <foo\+0x4>
    210c:	b4fff800 	cbz	x0, 200c <__rt_psrelocs_end\+0xc>
    2110:	b4000880 	cbz	x0, 2220 <bar>
    2114:	b4000880 	cbz	x0, 2224 <bar\+0x4>
    2118:	b4000820 	cbz	x0, 221c <.text>
    211c:	b4fff700 	cbz	x0, 1ffc <__ImageBase\+0xffc>
    2120:	3607f780 	tbz	w0, #0, 2010 <foo>
    2124:	3607f780 	tbz	w0, #0, 2014 <foo\+0x4>
    2128:	3607f720 	tbz	w0, #0, 200c <__rt_psrelocs_end\+0xc>
    212c:	360007a0 	tbz	w0, #0, 2220 <bar>
    2130:	360007a0 	tbz	w0, #0, 2224 <bar\+0x4>
    2134:	36000740 	tbz	w0, #0, 221c <.text>
    2138:	3607f620 	tbz	w0, #0, 1ffc <__ImageBase\+0xffc>
    213c:	90000000 	adrp	x0, 2000 <__rt_psrelocs_end>
    2140:	90000000 	adrp	x0, 2000 <__rt_psrelocs_end>
    2144:	90000000 	adrp	x0, 2000 <__rt_psrelocs_end>
    2148:	90000000 	adrp	x0, 2000 <__rt_psrelocs_end>
    214c:	90000000 	adrp	x0, 2000 <__rt_psrelocs_end>
    2150:	90000000 	adrp	x0, 2000 <__rt_psrelocs_end>
    2154:	f0ffffe0 	adrp	x0, 1000 <__ImageBase>
    2158:	10fff5c0 	adr	x0, 2010 <foo>
    215c:	30fff5a0 	adr	x0, 2011 <foo\+0x1>
    2160:	70fff560 	adr	x0, 200f <__rt_psrelocs_end\+0xf>
    2164:	100005e0 	adr	x0, 2220 <bar>
    2168:	300005c0 	adr	x0, 2221 <bar\+0x1>
    216c:	70000580 	adr	x0, 221f <.text\+0x3>
    2170:	70fff460 	adr	x0, 1fff <__ImageBase\+0xfff>
    2174:	39004000 	strb	w0, \[x0, #16\]
    2178:	39005000 	strb	w0, \[x0, #20\]
    217c:	39003000 	strb	w0, \[x0, #12\]
    2180:	39088000 	strb	w0, \[x0, #544\]
    2184:	39089000 	strb	w0, \[x0, #548\]
    2188:	39087000 	strb	w0, \[x0, #540\]
    218c:	393ff000 	strb	w0, \[x0, #4092\]
    2190:	79002000 	strh	w0, \[x0, #16\]
    2194:	79002800 	strh	w0, \[x0, #20\]
    2198:	79001800 	strh	w0, \[x0, #12\]
    219c:	79044000 	strh	w0, \[x0, #544\]
    21a0:	79044800 	strh	w0, \[x0, #548\]
    21a4:	79043800 	strh	w0, \[x0, #540\]
    21a8:	791ff800 	strh	w0, \[x0, #4092\]
    21ac:	b9001000 	str	w0, \[x0, #16\]
    21b0:	b9001400 	str	w0, \[x0, #20\]
    21b4:	b9000c00 	str	w0, \[x0, #12\]
    21b8:	b9022000 	str	w0, \[x0, #544\]
    21bc:	b9022400 	str	w0, \[x0, #548\]
    21c0:	b9021c00 	str	w0, \[x0, #540\]
    21c4:	b90ffc00 	str	w0, \[x0, #4092\]
    21c8:	f9000800 	str	x0, \[x0, #16\]
    21cc:	f9000c00 	str	x0, \[x0, #24\]
    21d0:	f9000400 	str	x0, \[x0, #8\]
    21d4:	f9011000 	str	x0, \[x0, #544\]
    21d8:	f9011400 	str	x0, \[x0, #552\]
    21dc:	f9010c00 	str	x0, \[x0, #536\]
    21e0:	f907fc00 	str	x0, \[x0, #4088\]
    21e4:	3d800400 	str	q0, \[x0, #16\]
    21e8:	3d800800 	str	q0, \[x0, #32\]
    21ec:	3d800000 	str	q0, \[x0\]
    21f0:	3d808800 	str	q0, \[x0, #544\]
    21f4:	3d808c00 	str	q0, \[x0, #560\]
    21f8:	3d808400 	str	q0, \[x0, #528\]
    21fc:	3d83fc00 	str	q0, \[x0, #4080\]
    2200:	91004000 	add	x0, x0, #0x10
    2204:	91004400 	add	x0, x0, #0x11
    2208:	91003c00 	add	x0, x0, #0xf
    220c:	91088000 	add	x0, x0, #0x220
    2210:	91088400 	add	x0, x0, #0x221
    2214:	91087c00 	add	x0, x0, #0x21f
    2218:	913ffc00 	add	x0, x0, #0xfff

000000000000221c <.text>:
    221c:	00000000 	udf	#0

0000000000002220 <bar>:
    2220:	9abcdef0 	.inst	0x9abcdef0 ; undefined
    2224:	12345678 	and	w24, w19, #0xfffff003

0000000000002228 <__CTOR_LIST__>:
    2228:	ffffffff 	.inst	0xffffffff ; undefined
    222c:	ffffffff 	.inst	0xffffffff ; undefined
	...

0000000000002238 <__DTOR_LIST__>:
    2238:	ffffffff 	.inst	0xffffffff ; undefined
    223c:	ffffffff 	.inst	0xffffffff ; undefined
	...
