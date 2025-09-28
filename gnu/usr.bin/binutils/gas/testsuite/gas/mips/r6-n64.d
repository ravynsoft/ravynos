#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS MIPSR6 instructions
#as: -64
#source: r6.s

# Check MIPSR6 instructions

.*: +file format .*mips.*

Disassembly of section .text:
0+0000 <[^>]*> 46020818 	maddf.s	\$f0,\$f1,\$f2
0+0004 <[^>]*> 462520d8 	maddf.d	\$f3,\$f4,\$f5
0+0008 <[^>]*> 46083999 	msubf.s	\$f6,\$f7,\$f8
0+000c <[^>]*> 462b5259 	msubf.d	\$f9,\$f10,\$f11
0+0010 <[^>]*> 46820800 	cmp.af.s	\$f0,\$f1,\$f2
0+0014 <[^>]*> 46a20800 	cmp.af.d	\$f0,\$f1,\$f2
0+0018 <[^>]*> 46820801 	cmp.un.s	\$f0,\$f1,\$f2
0+001c <[^>]*> 46a20801 	cmp.un.d	\$f0,\$f1,\$f2
0+0020 <[^>]*> 46820802 	cmp.eq.s	\$f0,\$f1,\$f2
0+0024 <[^>]*> 46a20802 	cmp.eq.d	\$f0,\$f1,\$f2
0+0028 <[^>]*> 46820803 	cmp.ueq.s	\$f0,\$f1,\$f2
0+002c <[^>]*> 46a20803 	cmp.ueq.d	\$f0,\$f1,\$f2
0+0030 <[^>]*> 46820804 	cmp.lt.s	\$f0,\$f1,\$f2
0+0034 <[^>]*> 46a20804 	cmp.lt.d	\$f0,\$f1,\$f2
0+0038 <[^>]*> 46820805 	cmp.ult.s	\$f0,\$f1,\$f2
0+003c <[^>]*> 46a20805 	cmp.ult.d	\$f0,\$f1,\$f2
0+0040 <[^>]*> 46820806 	cmp.le.s	\$f0,\$f1,\$f2
0+0044 <[^>]*> 46a20806 	cmp.le.d	\$f0,\$f1,\$f2
0+0048 <[^>]*> 46820807 	cmp.ule.s	\$f0,\$f1,\$f2
0+004c <[^>]*> 46a20807 	cmp.ule.d	\$f0,\$f1,\$f2
0+0050 <[^>]*> 46820808 	cmp.saf.s	\$f0,\$f1,\$f2
0+0054 <[^>]*> 46a20808 	cmp.saf.d	\$f0,\$f1,\$f2
0+0058 <[^>]*> 46820809 	cmp.sun.s	\$f0,\$f1,\$f2
0+005c <[^>]*> 46a20809 	cmp.sun.d	\$f0,\$f1,\$f2
0+0060 <[^>]*> 4682080a 	cmp.seq.s	\$f0,\$f1,\$f2
0+0064 <[^>]*> 46a2080a 	cmp.seq.d	\$f0,\$f1,\$f2
0+0068 <[^>]*> 4682080b 	cmp.sueq.s	\$f0,\$f1,\$f2
0+006c <[^>]*> 46a2080b 	cmp.sueq.d	\$f0,\$f1,\$f2
0+0070 <[^>]*> 4682080c 	cmp.slt.s	\$f0,\$f1,\$f2
0+0074 <[^>]*> 46a2080c 	cmp.slt.d	\$f0,\$f1,\$f2
0+0078 <[^>]*> 4682080d 	cmp.sult.s	\$f0,\$f1,\$f2
0+007c <[^>]*> 46a2080d 	cmp.sult.d	\$f0,\$f1,\$f2
0+0080 <[^>]*> 4682080e 	cmp.sle.s	\$f0,\$f1,\$f2
0+0084 <[^>]*> 46a2080e 	cmp.sle.d	\$f0,\$f1,\$f2
0+0088 <[^>]*> 4682080f 	cmp.sule.s	\$f0,\$f1,\$f2
0+008c <[^>]*> 46a2080f 	cmp.sule.d	\$f0,\$f1,\$f2
0+0090 <[^>]*> 46820811 	cmp.or.s	\$f0,\$f1,\$f2
0+0094 <[^>]*> 46a20811 	cmp.or.d	\$f0,\$f1,\$f2
0+0098 <[^>]*> 46820812 	cmp.une.s	\$f0,\$f1,\$f2
0+009c <[^>]*> 46a20812 	cmp.une.d	\$f0,\$f1,\$f2
0+00a0 <[^>]*> 46820813 	cmp.ne.s	\$f0,\$f1,\$f2
0+00a4 <[^>]*> 46a20813 	cmp.ne.d	\$f0,\$f1,\$f2
0+00a8 <[^>]*> 46820819 	cmp.sor.s	\$f0,\$f1,\$f2
0+00ac <[^>]*> 46a20819 	cmp.sor.d	\$f0,\$f1,\$f2
0+00b0 <[^>]*> 4682081a 	cmp.sune.s	\$f0,\$f1,\$f2
0+00b4 <[^>]*> 46a2081a 	cmp.sune.d	\$f0,\$f1,\$f2
0+00b8 <[^>]*> 4682081b 	cmp.sne.s	\$f0,\$f1,\$f2
0+00bc <[^>]*> 46a2081b 	cmp.sne.d	\$f0,\$f1,\$f2
0+00c0 <[^>]*> 45200000 	bc1eqz	\$f0,0+00c4 <[^>]*>
[	]*c0: R_MIPS_PC16	.L1.*1-0x4
[	]*c0: R_MIPS_NONE	\*ABS\*-0x4
[	]*c0: R_MIPS_NONE	\*ABS\*-0x4
0+00c4 <[^>]*> 00000000 	nop
0+00c8 <[^>]*> 453f0000 	bc1eqz	\$f31,0+00cc <[^>]*>
[	]*c8: R_MIPS_PC16	.L1.*1-0x4
[	]*c8: R_MIPS_NONE	\*ABS\*-0x4
[	]*c8: R_MIPS_NONE	\*ABS\*-0x4
0+00cc <[^>]*> 00000000 	nop
0+00d0 <[^>]*> 453f0000 	bc1eqz	\$f31,0+00d4 <[^>]*>
[	]*d0: R_MIPS_PC16	new-0x4
[	]*d0: R_MIPS_NONE	\*ABS\*-0x4
[	]*d0: R_MIPS_NONE	\*ABS\*-0x4
0+00d4 <[^>]*> 00000000 	nop
0+00d8 <[^>]*> 453f0000 	bc1eqz	\$f31,0+00dc <[^>]*>
[	]*d8: R_MIPS_PC16	external_label-0x4
[	]*d8: R_MIPS_NONE	\*ABS\*-0x4
[	]*d8: R_MIPS_NONE	\*ABS\*-0x4
0+00dc <[^>]*> 00000000 	nop
0+00e0 <[^>]*> 45a00000 	bc1nez	\$f0,0+00e4 <[^>]*>
[	]*e0: R_MIPS_PC16	.L1.*1-0x4
[	]*e0: R_MIPS_NONE	\*ABS\*-0x4
[	]*e0: R_MIPS_NONE	\*ABS\*-0x4
0+00e4 <[^>]*> 00000000 	nop
0+00e8 <[^>]*> 45bf0000 	bc1nez	\$f31,0+00ec <[^>]*>
[	]*e8: R_MIPS_PC16	.L1.*1-0x4
[	]*e8: R_MIPS_NONE	\*ABS\*-0x4
[	]*e8: R_MIPS_NONE	\*ABS\*-0x4
0+00ec <[^>]*> 00000000 	nop
0+00f0 <[^>]*> 45bf0000 	bc1nez	\$f31,0+00f4 <[^>]*>
[	]*f0: R_MIPS_PC16	new-0x4
[	]*f0: R_MIPS_NONE	\*ABS\*-0x4
[	]*f0: R_MIPS_NONE	\*ABS\*-0x4
0+00f4 <[^>]*> 00000000 	nop
0+00f8 <[^>]*> 45bf0000 	bc1nez	\$f31,0+00fc <[^>]*>
[	]*f8: R_MIPS_PC16	external_label-0x4
[	]*f8: R_MIPS_NONE	\*ABS\*-0x4
[	]*f8: R_MIPS_NONE	\*ABS\*-0x4
0+00fc <[^>]*> 00000000 	nop
0+0100 <[^>]*> 49200000 	bc2eqz	\$0,0+0104 <[^>]*>
[	]*100: R_MIPS_PC16	.L1.*1-0x4
[	]*100: R_MIPS_NONE	\*ABS\*-0x4
[	]*100: R_MIPS_NONE	\*ABS\*-0x4
0+0104 <[^>]*> 00000000 	nop
0+0108 <[^>]*> 493f0000 	bc2eqz	\$31,0+010c <[^>]*>
[	]*108: R_MIPS_PC16	.L1.*1-0x4
[	]*108: R_MIPS_NONE	\*ABS\*-0x4
[	]*108: R_MIPS_NONE	\*ABS\*-0x4
0+010c <[^>]*> 00000000 	nop
0+0110 <[^>]*> 493f0000 	bc2eqz	\$31,0+0114 <[^>]*>
[	]*110: R_MIPS_PC16	new-0x4
[	]*110: R_MIPS_NONE	\*ABS\*-0x4
[	]*110: R_MIPS_NONE	\*ABS\*-0x4
0+0114 <[^>]*> 00000000 	nop
0+0118 <[^>]*> 493f0000 	bc2eqz	\$31,0+011c <[^>]*>
[	]*118: R_MIPS_PC16	external_label-0x4
[	]*118: R_MIPS_NONE	\*ABS\*-0x4
[	]*118: R_MIPS_NONE	\*ABS\*-0x4
0+011c <[^>]*> 00000000 	nop
0+0120 <[^>]*> 49a00000 	bc2nez	\$0,0+0124 <[^>]*>
[	]*120: R_MIPS_PC16	.L1.*1-0x4
[	]*120: R_MIPS_NONE	\*ABS\*-0x4
[	]*120: R_MIPS_NONE	\*ABS\*-0x4
0+0124 <[^>]*> 00000000 	nop
0+0128 <[^>]*> 49bf0000 	bc2nez	\$31,0+012c <[^>]*>
[	]*128: R_MIPS_PC16	.L1.*1-0x4
[	]*128: R_MIPS_NONE	\*ABS\*-0x4
[	]*128: R_MIPS_NONE	\*ABS\*-0x4
0+012c <[^>]*> 00000000 	nop
0+0130 <[^>]*> 49bf0000 	bc2nez	\$31,0+0134 <[^>]*>
[	]*130: R_MIPS_PC16	new-0x4
[	]*130: R_MIPS_NONE	\*ABS\*-0x4
[	]*130: R_MIPS_NONE	\*ABS\*-0x4
0+0134 <[^>]*> 00000000 	nop
0+0138 <[^>]*> 49bf0000 	bc2nez	\$31,0+013c <[^>]*>
[	]*138: R_MIPS_PC16	external_label-0x4
[	]*138: R_MIPS_NONE	\*ABS\*-0x4
[	]*138: R_MIPS_NONE	\*ABS\*-0x4
0+013c <[^>]*> 00000000 	nop
0+0140 <[^>]*> 46020810 	sel.s	\$f0,\$f1,\$f2
0+0144 <[^>]*> 46220810 	sel.d	\$f0,\$f1,\$f2
0+0148 <[^>]*> 46020814 	seleqz.s	\$f0,\$f1,\$f2
0+014c <[^>]*> 46220814 	seleqz.d	\$f0,\$f1,\$f2
0+0150 <[^>]*> 46020817 	selnez.s	\$f0,\$f1,\$f2
0+0154 <[^>]*> 46220817 	selnez.d	\$f0,\$f1,\$f2
0+0158 <[^>]*> 00641035 	seleqz	v0,v1,a0
0+015c <[^>]*> 00641037 	selnez	v0,v1,a0
0+0160 <[^>]*> 00641098 	mul	v0,v1,a0
0+0164 <[^>]*> 006410d8 	muh	v0,v1,a0
0+0168 <[^>]*> 00641099 	mulu	v0,v1,a0
0+016c <[^>]*> 006410d9 	muhu	v0,v1,a0
0+0170 <[^>]*> 0064109a 	div	v0,v1,a0
0+0174 <[^>]*> 006410da 	mod	v0,v1,a0
0+0178 <[^>]*> 0064109b 	divu	v0,v1,a0
0+017c <[^>]*> 006410db 	modu	v0,v1,a0
0+0180 <[^>]*> 49422000 	lwc2	\$2,0\(a0\)
0+0184 <[^>]*> 49422400 	lwc2	\$2,-1024\(a0\)
0+0188 <[^>]*> 494223ff 	lwc2	\$2,1023\(a0\)
0+018c <[^>]*> 49622000 	swc2	\$2,0\(a0\)
0+0190 <[^>]*> 49622400 	swc2	\$2,-1024\(a0\)
0+0194 <[^>]*> 496223ff 	swc2	\$2,1023\(a0\)
0+0198 <[^>]*> 49c22000 	ldc2	\$2,0\(a0\)
0+019c <[^>]*> 49c22400 	ldc2	\$2,-1024\(a0\)
0+01a0 <[^>]*> 49c223ff 	ldc2	\$2,1023\(a0\)
0+01a4 <[^>]*> 49e22000 	sdc2	\$2,0\(a0\)
0+01a8 <[^>]*> 49e22400 	sdc2	\$2,-1024\(a0\)
0+01ac <[^>]*> 49e223ff 	sdc2	\$2,1023\(a0\)
0+01b0 <[^>]*> 00641005 	lsa	v0,v1,a0,0x1
0+01b4 <[^>]*> 006410c5 	lsa	v0,v1,a0,0x4
0+01b8 <[^>]*> 00601050 	clz	v0,v1
0+01bc <[^>]*> 00601051 	clo	v0,v1
0+01c0 <[^>]*> 0000000e 	sdbbp
0+01c4 <[^>]*> 0000000e 	sdbbp
0+01c8 <[^>]*> 0000004e 	sdbbp	0x1
0+01cc <[^>]*> 03ffffce 	sdbbp	0xfffff
0+01d0 <[^>]*> 3c02ffff 	lui	v0,0xffff
0+01d4 <[^>]*> 7c008035 	pref	0x0,-256\(zero\)
0+01d8 <[^>]*> 7fff7fb5 	pref	0x1f,255\(ra\)
0+01dc <[^>]*> 7c628036 	ll	v0,-256\(v1\)
0+01e0 <[^>]*> 7c627fb6 	ll	v0,255\(v1\)
0+01e4 <[^>]*> 7c628026 	sc	v0,-256\(v1\)
0+01e8 <[^>]*> 7c627fa6 	sc	v0,255\(v1\)
0+01ec <[^>]*> 7c608025 	cache	0x0,-256\(v1\)
0+01f0 <[^>]*> 7c7f7fa5 	cache	0x1f,255\(v1\)
0+01f4 <[^>]*> 7c432220 	align	a0,v0,v1,0
0+01f8 <[^>]*> 7c432260 	align	a0,v0,v1,1
0+01fc <[^>]*> 7c4322a0 	align	a0,v0,v1,2
0+0200 <[^>]*> 7c4322e0 	align	a0,v0,v1,3
0+0204 <[^>]*> 7c022020 	bitswap	a0,v0
0+0208 <[^>]*> 20000000 	bovc	zero,zero,0+020c <[^>]*>
[	]*208: R_MIPS_PC16	ext-0x4
[	]*208: R_MIPS_NONE	\*ABS\*-0x4
[	]*208: R_MIPS_NONE	\*ABS\*-0x4
0+020c <[^>]*> 00000000 	nop
0+0210 <[^>]*> 20400000 	bovc	v0,zero,0+0214 <[^>]*>
[	]*210: R_MIPS_PC16	ext-0x4
[	]*210: R_MIPS_NONE	\*ABS\*-0x4
[	]*210: R_MIPS_NONE	\*ABS\*-0x4
0+0214 <[^>]*> 00000000 	nop
0+0218 <[^>]*> 20400000 	bovc	v0,zero,0+021c <[^>]*>
[	]*218: R_MIPS_PC16	ext-0x4
[	]*218: R_MIPS_NONE	\*ABS\*-0x4
[	]*218: R_MIPS_NONE	\*ABS\*-0x4
0+021c <[^>]*> 00000000 	nop
0+0220 <[^>]*> 20820000 	bovc	a0,v0,0+0224 <[^>]*>
[	]*220: R_MIPS_PC16	ext-0x4
[	]*220: R_MIPS_NONE	\*ABS\*-0x4
[	]*220: R_MIPS_NONE	\*ABS\*-0x4
0+0224 <[^>]*> 00000000 	nop
0+0228 <[^>]*> 20820000 	bovc	a0,v0,0+022c <[^>]*>
[	]*228: R_MIPS_PC16	ext-0x4
[	]*228: R_MIPS_NONE	\*ABS\*-0x4
[	]*228: R_MIPS_NONE	\*ABS\*-0x4
0+022c <[^>]*> 00000000 	nop
0+0230 <[^>]*> 20820000 	bovc	a0,v0,0+0234 <[^>]*>
[	]*230: R_MIPS_PC16	L0.*-0x20000
[	]*230: R_MIPS_NONE	\*ABS\*-0x20000
[	]*230: R_MIPS_NONE	\*ABS\*-0x20000
0+0234 <[^>]*> 00000000 	nop
0+0238 <[^>]*> 20820000 	bovc	a0,v0,0+023c <[^>]*>
[	]*238: R_MIPS_PC16	L0.*\+0x1fffc
[	]*238: R_MIPS_NONE	\*ABS\*\+0x1fffc
[	]*238: R_MIPS_NONE	\*ABS\*\+0x1fffc
0+023c <[^>]*> 00000000 	nop
0+0240 <[^>]*> 20820000 	bovc	a0,v0,0+0244 <[^>]*>
[	]*240: R_MIPS_PC16	.L1.*2-0x4
[	]*240: R_MIPS_NONE	\*ABS\*-0x4
[	]*240: R_MIPS_NONE	\*ABS\*-0x4
0+0244 <[^>]*> 00000000 	nop
0+0248 <[^>]*> 20420000 	bovc	v0,v0,0+024c <[^>]*>
[	]*248: R_MIPS_PC16	ext-0x4
[	]*248: R_MIPS_NONE	\*ABS\*-0x4
[	]*248: R_MIPS_NONE	\*ABS\*-0x4
0+024c <[^>]*> 00000000 	nop
0+0250 <[^>]*> 20420000 	bovc	v0,v0,0+0254 <[^>]*>
[	]*250: R_MIPS_PC16	L0.*-0x20000
[	]*250: R_MIPS_NONE	\*ABS\*-0x20000
[	]*250: R_MIPS_NONE	\*ABS\*-0x20000
0+0254 <[^>]*> 00000000 	nop
0+0258 <[^>]*> 20020000 	beqzalc	v0,0+025c <[^>]*>
[	]*258: R_MIPS_PC16	ext-0x4
[	]*258: R_MIPS_NONE	\*ABS\*-0x4
[	]*258: R_MIPS_NONE	\*ABS\*-0x4
0+025c <[^>]*> 00000000 	nop
0+0260 <[^>]*> 20020000 	beqzalc	v0,0+0264 <[^>]*>
[	]*260: R_MIPS_PC16	L0.*-0x20000
[	]*260: R_MIPS_NONE	\*ABS\*-0x20000
[	]*260: R_MIPS_NONE	\*ABS\*-0x20000
0+0264 <[^>]*> 00000000 	nop
0+0268 <[^>]*> 20020000 	beqzalc	v0,0+026c <[^>]*>
[	]*268: R_MIPS_PC16	L0.*\+0x1fffc
[	]*268: R_MIPS_NONE	\*ABS\*\+0x1fffc
[	]*268: R_MIPS_NONE	\*ABS\*\+0x1fffc
0+026c <[^>]*> 00000000 	nop
0+0270 <[^>]*> 20020000 	beqzalc	v0,0+0274 <[^>]*>
[	]*270: R_MIPS_PC16	.L1.*2-0x4
[	]*270: R_MIPS_NONE	\*ABS\*-0x4
[	]*270: R_MIPS_NONE	\*ABS\*-0x4
0+0274 <[^>]*> 00000000 	nop
0+0278 <[^>]*> 20430000 	beqc	v0,v1,0+027c <[^>]*>
[	]*278: R_MIPS_PC16	ext-0x4
[	]*278: R_MIPS_NONE	\*ABS\*-0x4
[	]*278: R_MIPS_NONE	\*ABS\*-0x4
0+027c <[^>]*> 00000000 	nop
0+0280 <[^>]*> 20430000 	beqc	v0,v1,0+0284 <[^>]*>
[	]*280: R_MIPS_PC16	ext-0x4
[	]*280: R_MIPS_NONE	\*ABS\*-0x4
[	]*280: R_MIPS_NONE	\*ABS\*-0x4
0+0284 <[^>]*> 00000000 	nop
0+0288 <[^>]*> 20430000 	beqc	v0,v1,0+028c <[^>]*>
[	]*288: R_MIPS_PC16	L0.*-0x20000
[	]*288: R_MIPS_NONE	\*ABS\*-0x20000
[	]*288: R_MIPS_NONE	\*ABS\*-0x20000
0+028c <[^>]*> 00000000 	nop
0+0290 <[^>]*> 20430000 	beqc	v0,v1,0+0294 <[^>]*>
[	]*290: R_MIPS_PC16	L0.*\+0x1fffc
[	]*290: R_MIPS_NONE	\*ABS\*\+0x1fffc
[	]*290: R_MIPS_NONE	\*ABS\*\+0x1fffc
0+0294 <[^>]*> 00000000 	nop
0+0298 <[^>]*> 20430000 	beqc	v0,v1,0+029c <[^>]*>
[	]*298: R_MIPS_PC16	.L1.*2-0x4
[	]*298: R_MIPS_NONE	\*ABS\*-0x4
[	]*298: R_MIPS_NONE	\*ABS\*-0x4
0+029c <[^>]*> 00000000 	nop
0+02a0 <[^>]*> 60000000 	bnvc	zero,zero,0+02a4 <[^>]*>
[	]*2a0: R_MIPS_PC16	ext-0x4
[	]*2a0: R_MIPS_NONE	\*ABS\*-0x4
[	]*2a0: R_MIPS_NONE	\*ABS\*-0x4
0+02a4 <[^>]*> 00000000 	nop
0+02a8 <[^>]*> 60400000 	bnvc	v0,zero,0+02ac <[^>]*>
[	]*2a8: R_MIPS_PC16	ext-0x4
[	]*2a8: R_MIPS_NONE	\*ABS\*-0x4
[	]*2a8: R_MIPS_NONE	\*ABS\*-0x4
0+02ac <[^>]*> 00000000 	nop
0+02b0 <[^>]*> 60400000 	bnvc	v0,zero,0+02b4 <[^>]*>
[	]*2b0: R_MIPS_PC16	ext-0x4
[	]*2b0: R_MIPS_NONE	\*ABS\*-0x4
[	]*2b0: R_MIPS_NONE	\*ABS\*-0x4
0+02b4 <[^>]*> 00000000 	nop
0+02b8 <[^>]*> 60820000 	bnvc	a0,v0,0+02bc <[^>]*>
[	]*2b8: R_MIPS_PC16	ext-0x4
[	]*2b8: R_MIPS_NONE	\*ABS\*-0x4
[	]*2b8: R_MIPS_NONE	\*ABS\*-0x4
0+02bc <[^>]*> 00000000 	nop
0+02c0 <[^>]*> 60820000 	bnvc	a0,v0,0+02c4 <[^>]*>
[	]*2c0: R_MIPS_PC16	ext-0x4
[	]*2c0: R_MIPS_NONE	\*ABS\*-0x4
[	]*2c0: R_MIPS_NONE	\*ABS\*-0x4
0+02c4 <[^>]*> 00000000 	nop
0+02c8 <[^>]*> 60820000 	bnvc	a0,v0,0+02cc <[^>]*>
[	]*2c8: R_MIPS_PC16	L0.*-0x20000
[	]*2c8: R_MIPS_NONE	\*ABS\*-0x20000
[	]*2c8: R_MIPS_NONE	\*ABS\*-0x20000
0+02cc <[^>]*> 00000000 	nop
0+02d0 <[^>]*> 60820000 	bnvc	a0,v0,0+02d4 <[^>]*>
[	]*2d0: R_MIPS_PC16	L0.*\+0x1fffc
[	]*2d0: R_MIPS_NONE	\*ABS\*\+0x1fffc
[	]*2d0: R_MIPS_NONE	\*ABS\*\+0x1fffc
0+02d4 <[^>]*> 00000000 	nop
0+02d8 <[^>]*> 60820000 	bnvc	a0,v0,0+02dc <[^>]*>
[	]*2d8: R_MIPS_PC16	.L1.*2-0x4
[	]*2d8: R_MIPS_NONE	\*ABS\*-0x4
[	]*2d8: R_MIPS_NONE	\*ABS\*-0x4
0+02dc <[^>]*> 00000000 	nop
0+02e0 <[^>]*> 60420000 	bnvc	v0,v0,0+02e4 <[^>]*>
[	]*2e0: R_MIPS_PC16	ext-0x4
[	]*2e0: R_MIPS_NONE	\*ABS\*-0x4
[	]*2e0: R_MIPS_NONE	\*ABS\*-0x4
0+02e4 <[^>]*> 00000000 	nop
0+02e8 <[^>]*> 60420000 	bnvc	v0,v0,0+02ec <[^>]*>
[	]*2e8: R_MIPS_PC16	L0.*-0x20000
[	]*2e8: R_MIPS_NONE	\*ABS\*-0x20000
[	]*2e8: R_MIPS_NONE	\*ABS\*-0x20000
0+02ec <[^>]*> 00000000 	nop
0+02f0 <[^>]*> 60020000 	bnezalc	v0,0+02f4 <[^>]*>
[	]*2f0: R_MIPS_PC16	ext-0x4
[	]*2f0: R_MIPS_NONE	\*ABS\*-0x4
[	]*2f0: R_MIPS_NONE	\*ABS\*-0x4
0+02f4 <[^>]*> 00000000 	nop
0+02f8 <[^>]*> 60020000 	bnezalc	v0,0+02fc <[^>]*>
[	]*2f8: R_MIPS_PC16	L0.*-0x20000
[	]*2f8: R_MIPS_NONE	\*ABS\*-0x20000
[	]*2f8: R_MIPS_NONE	\*ABS\*-0x20000
0+02fc <[^>]*> 00000000 	nop
0+0300 <[^>]*> 60020000 	bnezalc	v0,0+0304 <[^>]*>
[	]*300: R_MIPS_PC16	L0.*\+0x1fffc
[	]*300: R_MIPS_NONE	\*ABS\*\+0x1fffc
[	]*300: R_MIPS_NONE	\*ABS\*\+0x1fffc
0+0304 <[^>]*> 00000000 	nop
0+0308 <[^>]*> 60020000 	bnezalc	v0,0+030c <[^>]*>
[	]*308: R_MIPS_PC16	.L1.*2-0x4
[	]*308: R_MIPS_NONE	\*ABS\*-0x4
[	]*308: R_MIPS_NONE	\*ABS\*-0x4
0+030c <[^>]*> 00000000 	nop
0+0310 <[^>]*> 60430000 	bnec	v0,v1,0+0314 <[^>]*>
[	]*310: R_MIPS_PC16	ext-0x4
[	]*310: R_MIPS_NONE	\*ABS\*-0x4
[	]*310: R_MIPS_NONE	\*ABS\*-0x4
0+0314 <[^>]*> 00000000 	nop
0+0318 <[^>]*> 60430000 	bnec	v0,v1,0+031c <[^>]*>
[	]*318: R_MIPS_PC16	ext-0x4
[	]*318: R_MIPS_NONE	\*ABS\*-0x4
[	]*318: R_MIPS_NONE	\*ABS\*-0x4
0+031c <[^>]*> 00000000 	nop
0+0320 <[^>]*> 60430000 	bnec	v0,v1,0+0324 <[^>]*>
[	]*320: R_MIPS_PC16	L0.*-0x20000
[	]*320: R_MIPS_NONE	\*ABS\*-0x20000
[	]*320: R_MIPS_NONE	\*ABS\*-0x20000
0+0324 <[^>]*> 00000000 	nop
0+0328 <[^>]*> 60430000 	bnec	v0,v1,0+032c <[^>]*>
[	]*328: R_MIPS_PC16	L0.*\+0x1fffc
[	]*328: R_MIPS_NONE	\*ABS\*\+0x1fffc
[	]*328: R_MIPS_NONE	\*ABS\*\+0x1fffc
0+032c <[^>]*> 00000000 	nop
0+0330 <[^>]*> 60430000 	bnec	v0,v1,0+0334 <[^>]*>
[	]*330: R_MIPS_PC16	.L1.*2-0x4
[	]*330: R_MIPS_NONE	\*ABS\*-0x4
[	]*330: R_MIPS_NONE	\*ABS\*-0x4
0+0334 <[^>]*> 00000000 	nop
0+0338 <[^>]*> 58020000 	blezc	v0,0+033c <[^>]*>
[	]*338: R_MIPS_PC16	ext-0x4
[	]*338: R_MIPS_NONE	\*ABS\*-0x4
[	]*338: R_MIPS_NONE	\*ABS\*-0x4
0+033c <[^>]*> 00000000 	nop
0+0340 <[^>]*> 58020000 	blezc	v0,0+0344 <[^>]*>
[	]*340: R_MIPS_PC16	L0.*-0x20000
[	]*340: R_MIPS_NONE	\*ABS\*-0x20000
[	]*340: R_MIPS_NONE	\*ABS\*-0x20000
0+0344 <[^>]*> 00000000 	nop
0+0348 <[^>]*> 58020000 	blezc	v0,0+034c <[^>]*>
[	]*348: R_MIPS_PC16	L0.*\+0x1fffc
[	]*348: R_MIPS_NONE	\*ABS\*\+0x1fffc
[	]*348: R_MIPS_NONE	\*ABS\*\+0x1fffc
0+034c <[^>]*> 00000000 	nop
0+0350 <[^>]*> 58020000 	blezc	v0,0+0354 <[^>]*>
[	]*350: R_MIPS_PC16	.L1.*2-0x4
[	]*350: R_MIPS_NONE	\*ABS\*-0x4
[	]*350: R_MIPS_NONE	\*ABS\*-0x4
0+0354 <[^>]*> 00000000 	nop
0+0358 <[^>]*> 58420000 	bgezc	v0,0+035c <[^>]*>
[	]*358: R_MIPS_PC16	ext-0x4
[	]*358: R_MIPS_NONE	\*ABS\*-0x4
[	]*358: R_MIPS_NONE	\*ABS\*-0x4
0+035c <[^>]*> 00000000 	nop
0+0360 <[^>]*> 58420000 	bgezc	v0,0+0364 <[^>]*>
[	]*360: R_MIPS_PC16	L0.*-0x20000
[	]*360: R_MIPS_NONE	\*ABS\*-0x20000
[	]*360: R_MIPS_NONE	\*ABS\*-0x20000
0+0364 <[^>]*> 00000000 	nop
0+0368 <[^>]*> 58420000 	bgezc	v0,0+036c <[^>]*>
[	]*368: R_MIPS_PC16	L0.*\+0x1fffc
[	]*368: R_MIPS_NONE	\*ABS\*\+0x1fffc
[	]*368: R_MIPS_NONE	\*ABS\*\+0x1fffc
0+036c <[^>]*> 00000000 	nop
0+0370 <[^>]*> 58420000 	bgezc	v0,0+0374 <[^>]*>
[	]*370: R_MIPS_PC16	.L1.*2-0x4
[	]*370: R_MIPS_NONE	\*ABS\*-0x4
[	]*370: R_MIPS_NONE	\*ABS\*-0x4
0+0374 <[^>]*> 00000000 	nop
0+0378 <[^>]*> 58430000 	bgec	v0,v1,0+037c <[^>]*>
[	]*378: R_MIPS_PC16	ext-0x4
[	]*378: R_MIPS_NONE	\*ABS\*-0x4
[	]*378: R_MIPS_NONE	\*ABS\*-0x4
0+037c <[^>]*> 00000000 	nop
0+0380 <[^>]*> 58430000 	bgec	v0,v1,0+0384 <[^>]*>
[	]*380: R_MIPS_PC16	L0.*-0x20000
[	]*380: R_MIPS_NONE	\*ABS\*-0x20000
[	]*380: R_MIPS_NONE	\*ABS\*-0x20000
0+0384 <[^>]*> 00000000 	nop
0+0388 <[^>]*> 58430000 	bgec	v0,v1,0+038c <[^>]*>
[	]*388: R_MIPS_PC16	L0.*\+0x1fffc
[	]*388: R_MIPS_NONE	\*ABS\*\+0x1fffc
[	]*388: R_MIPS_NONE	\*ABS\*\+0x1fffc
0+038c <[^>]*> 00000000 	nop
0+0390 <[^>]*> 58430000 	bgec	v0,v1,0+0394 <[^>]*>
[	]*390: R_MIPS_PC16	.L1.*2-0x4
[	]*390: R_MIPS_NONE	\*ABS\*-0x4
[	]*390: R_MIPS_NONE	\*ABS\*-0x4
0+0394 <[^>]*> 00000000 	nop
0+0398 <[^>]*> 58620000 	bgec	v1,v0,0+039c <[^>]*>
[	]*398: R_MIPS_PC16	.L1.*2-0x4
[	]*398: R_MIPS_NONE	\*ABS\*-0x4
[	]*398: R_MIPS_NONE	\*ABS\*-0x4
0+039c <[^>]*> 00000000 	nop
0+03a0 <[^>]*> 5c020000 	bgtzc	v0,0+03a4 <[^>]*>
[	]*3a0: R_MIPS_PC16	ext-0x4
[	]*3a0: R_MIPS_NONE	\*ABS\*-0x4
[	]*3a0: R_MIPS_NONE	\*ABS\*-0x4
0+03a4 <[^>]*> 00000000 	nop
0+03a8 <[^>]*> 5c020000 	bgtzc	v0,0+03ac <[^>]*>
[	]*3a8: R_MIPS_PC16	L0.*-0x20000
[	]*3a8: R_MIPS_NONE	\*ABS\*-0x20000
[	]*3a8: R_MIPS_NONE	\*ABS\*-0x20000
0+03ac <[^>]*> 00000000 	nop
0+03b0 <[^>]*> 5c020000 	bgtzc	v0,0+03b4 <[^>]*>
[	]*3b0: R_MIPS_PC16	L0.*\+0x1fffc
[	]*3b0: R_MIPS_NONE	\*ABS\*\+0x1fffc
[	]*3b0: R_MIPS_NONE	\*ABS\*\+0x1fffc
0+03b4 <[^>]*> 00000000 	nop
0+03b8 <[^>]*> 5c020000 	bgtzc	v0,0+03bc <[^>]*>
[	]*3b8: R_MIPS_PC16	.L1.*2-0x4
[	]*3b8: R_MIPS_NONE	\*ABS\*-0x4
[	]*3b8: R_MIPS_NONE	\*ABS\*-0x4
0+03bc <[^>]*> 00000000 	nop
0+03c0 <[^>]*> 5c420000 	bltzc	v0,0+03c4 <[^>]*>
[	]*3c0: R_MIPS_PC16	ext-0x4
[	]*3c0: R_MIPS_NONE	\*ABS\*-0x4
[	]*3c0: R_MIPS_NONE	\*ABS\*-0x4
0+03c4 <[^>]*> 00000000 	nop
0+03c8 <[^>]*> 5c420000 	bltzc	v0,0+03cc <[^>]*>
[	]*3c8: R_MIPS_PC16	L0.*-0x20000
[	]*3c8: R_MIPS_NONE	\*ABS\*-0x20000
[	]*3c8: R_MIPS_NONE	\*ABS\*-0x20000
0+03cc <[^>]*> 00000000 	nop
0+03d0 <[^>]*> 5c420000 	bltzc	v0,0+03d4 <[^>]*>
[	]*3d0: R_MIPS_PC16	L0.*\+0x1fffc
[	]*3d0: R_MIPS_NONE	\*ABS\*\+0x1fffc
[	]*3d0: R_MIPS_NONE	\*ABS\*\+0x1fffc
0+03d4 <[^>]*> 00000000 	nop
0+03d8 <[^>]*> 5c420000 	bltzc	v0,0+03dc <[^>]*>
[	]*3d8: R_MIPS_PC16	.L1.*2-0x4
[	]*3d8: R_MIPS_NONE	\*ABS\*-0x4
[	]*3d8: R_MIPS_NONE	\*ABS\*-0x4
0+03dc <[^>]*> 00000000 	nop
0+03e0 <[^>]*> 5c430000 	bltc	v0,v1,0+03e4 <[^>]*>
[	]*3e0: R_MIPS_PC16	ext-0x4
[	]*3e0: R_MIPS_NONE	\*ABS\*-0x4
[	]*3e0: R_MIPS_NONE	\*ABS\*-0x4
0+03e4 <[^>]*> 00000000 	nop
0+03e8 <[^>]*> 5c430000 	bltc	v0,v1,0+03ec <[^>]*>
[	]*3e8: R_MIPS_PC16	L0.*-0x20000
[	]*3e8: R_MIPS_NONE	\*ABS\*-0x20000
[	]*3e8: R_MIPS_NONE	\*ABS\*-0x20000
0+03ec <[^>]*> 00000000 	nop
0+03f0 <[^>]*> 5c430000 	bltc	v0,v1,0+03f4 <[^>]*>
[	]*3f0: R_MIPS_PC16	L0.*\+0x1fffc
[	]*3f0: R_MIPS_NONE	\*ABS\*\+0x1fffc
[	]*3f0: R_MIPS_NONE	\*ABS\*\+0x1fffc
0+03f4 <[^>]*> 00000000 	nop
0+03f8 <[^>]*> 5c430000 	bltc	v0,v1,0+03fc <[^>]*>
[	]*3f8: R_MIPS_PC16	.L1.*2-0x4
[	]*3f8: R_MIPS_NONE	\*ABS\*-0x4
[	]*3f8: R_MIPS_NONE	\*ABS\*-0x4
0+03fc <[^>]*> 00000000 	nop
0+0400 <[^>]*> 5c620000 	bltc	v1,v0,0+0404 <[^>]*>
[	]*400: R_MIPS_PC16	.L1.*2-0x4
[	]*400: R_MIPS_NONE	\*ABS\*-0x4
[	]*400: R_MIPS_NONE	\*ABS\*-0x4
0+0404 <[^>]*> 00000000 	nop
0+0408 <[^>]*> 18020000 	blezalc	v0,0+040c <[^>]*>
[	]*408: R_MIPS_PC16	ext-0x4
[	]*408: R_MIPS_NONE	\*ABS\*-0x4
[	]*408: R_MIPS_NONE	\*ABS\*-0x4
0+040c <[^>]*> 00000000 	nop
0+0410 <[^>]*> 18020000 	blezalc	v0,0+0414 <[^>]*>
[	]*410: R_MIPS_PC16	L0.*-0x20000
[	]*410: R_MIPS_NONE	\*ABS\*-0x20000
[	]*410: R_MIPS_NONE	\*ABS\*-0x20000
0+0414 <[^>]*> 00000000 	nop
0+0418 <[^>]*> 18020000 	blezalc	v0,0+041c <[^>]*>
[	]*418: R_MIPS_PC16	L0.*\+0x1fffc
[	]*418: R_MIPS_NONE	\*ABS\*\+0x1fffc
[	]*418: R_MIPS_NONE	\*ABS\*\+0x1fffc
0+041c <[^>]*> 00000000 	nop
0+0420 <[^>]*> 18020000 	blezalc	v0,0+0424 <[^>]*>
[	]*420: R_MIPS_PC16	.L1.*2-0x4
[	]*420: R_MIPS_NONE	\*ABS\*-0x4
[	]*420: R_MIPS_NONE	\*ABS\*-0x4
0+0424 <[^>]*> 00000000 	nop
0+0428 <[^>]*> 18420000 	bgezalc	v0,0+042c <[^>]*>
[	]*428: R_MIPS_PC16	ext-0x4
[	]*428: R_MIPS_NONE	\*ABS\*-0x4
[	]*428: R_MIPS_NONE	\*ABS\*-0x4
0+042c <[^>]*> 00000000 	nop
0+0430 <[^>]*> 18420000 	bgezalc	v0,0+0434 <[^>]*>
[	]*430: R_MIPS_PC16	L0.*-0x20000
[	]*430: R_MIPS_NONE	\*ABS\*-0x20000
[	]*430: R_MIPS_NONE	\*ABS\*-0x20000
0+0434 <[^>]*> 00000000 	nop
0+0438 <[^>]*> 18420000 	bgezalc	v0,0+043c <[^>]*>
[	]*438: R_MIPS_PC16	L0.*\+0x1fffc
[	]*438: R_MIPS_NONE	\*ABS\*\+0x1fffc
[	]*438: R_MIPS_NONE	\*ABS\*\+0x1fffc
0+043c <[^>]*> 00000000 	nop
0+0440 <[^>]*> 18420000 	bgezalc	v0,0+0444 <[^>]*>
[	]*440: R_MIPS_PC16	.L1.*2-0x4
[	]*440: R_MIPS_NONE	\*ABS\*-0x4
[	]*440: R_MIPS_NONE	\*ABS\*-0x4
0+0444 <[^>]*> 00000000 	nop
0+0448 <[^>]*> 18430000 	bgeuc	v0,v1,0+044c <[^>]*>
[	]*448: R_MIPS_PC16	ext-0x4
[	]*448: R_MIPS_NONE	\*ABS\*-0x4
[	]*448: R_MIPS_NONE	\*ABS\*-0x4
0+044c <[^>]*> 00000000 	nop
0+0450 <[^>]*> 18430000 	bgeuc	v0,v1,0+0454 <[^>]*>
[	]*450: R_MIPS_PC16	L0.*-0x20000
[	]*450: R_MIPS_NONE	\*ABS\*-0x20000
[	]*450: R_MIPS_NONE	\*ABS\*-0x20000
0+0454 <[^>]*> 00000000 	nop
0+0458 <[^>]*> 18430000 	bgeuc	v0,v1,0+045c <[^>]*>
[	]*458: R_MIPS_PC16	L0.*\+0x1fffc
[	]*458: R_MIPS_NONE	\*ABS\*\+0x1fffc
[	]*458: R_MIPS_NONE	\*ABS\*\+0x1fffc
0+045c <[^>]*> 00000000 	nop
0+0460 <[^>]*> 18430000 	bgeuc	v0,v1,0+0464 <[^>]*>
[	]*460: R_MIPS_PC16	.L1.*2-0x4
[	]*460: R_MIPS_NONE	\*ABS\*-0x4
[	]*460: R_MIPS_NONE	\*ABS\*-0x4
0+0464 <[^>]*> 00000000 	nop
0+0468 <[^>]*> 18620000 	bgeuc	v1,v0,0+046c <[^>]*>
[	]*468: R_MIPS_PC16	.L1.*2-0x4
[	]*468: R_MIPS_NONE	\*ABS\*-0x4
[	]*468: R_MIPS_NONE	\*ABS\*-0x4
0+046c <[^>]*> 00000000 	nop
0+0470 <[^>]*> 1c020000 	bgtzalc	v0,0+0474 <[^>]*>
[	]*470: R_MIPS_PC16	ext-0x4
[	]*470: R_MIPS_NONE	\*ABS\*-0x4
[	]*470: R_MIPS_NONE	\*ABS\*-0x4
0+0474 <[^>]*> 00000000 	nop
0+0478 <[^>]*> 1c020000 	bgtzalc	v0,0+047c <[^>]*>
[	]*478: R_MIPS_PC16	L0.*-0x20000
[	]*478: R_MIPS_NONE	\*ABS\*-0x20000
[	]*478: R_MIPS_NONE	\*ABS\*-0x20000
0+047c <[^>]*> 00000000 	nop
0+0480 <[^>]*> 1c020000 	bgtzalc	v0,0+0484 <[^>]*>
[	]*480: R_MIPS_PC16	L0.*\+0x1fffc
[	]*480: R_MIPS_NONE	\*ABS\*\+0x1fffc
[	]*480: R_MIPS_NONE	\*ABS\*\+0x1fffc
0+0484 <[^>]*> 00000000 	nop
0+0488 <[^>]*> 1c020000 	bgtzalc	v0,0+048c <[^>]*>
[	]*488: R_MIPS_PC16	.L1.*2-0x4
[	]*488: R_MIPS_NONE	\*ABS\*-0x4
[	]*488: R_MIPS_NONE	\*ABS\*-0x4
0+048c <[^>]*> 00000000 	nop
0+0490 <[^>]*> 1c420000 	bltzalc	v0,0+0494 <[^>]*>
[	]*490: R_MIPS_PC16	ext-0x4
[	]*490: R_MIPS_NONE	\*ABS\*-0x4
[	]*490: R_MIPS_NONE	\*ABS\*-0x4
0+0494 <[^>]*> 00000000 	nop
0+0498 <[^>]*> 1c420000 	bltzalc	v0,0+049c <[^>]*>
[	]*498: R_MIPS_PC16	L0.*-0x20000
[	]*498: R_MIPS_NONE	\*ABS\*-0x20000
[	]*498: R_MIPS_NONE	\*ABS\*-0x20000
0+049c <[^>]*> 00000000 	nop
0+04a0 <[^>]*> 1c420000 	bltzalc	v0,0+04a4 <[^>]*>
[	]*4a0: R_MIPS_PC16	L0.*\+0x1fffc
[	]*4a0: R_MIPS_NONE	\*ABS\*\+0x1fffc
[	]*4a0: R_MIPS_NONE	\*ABS\*\+0x1fffc
0+04a4 <[^>]*> 00000000 	nop
0+04a8 <[^>]*> 1c420000 	bltzalc	v0,0+04ac <[^>]*>
[	]*4a8: R_MIPS_PC16	.L1.*2-0x4
[	]*4a8: R_MIPS_NONE	\*ABS\*-0x4
[	]*4a8: R_MIPS_NONE	\*ABS\*-0x4
0+04ac <[^>]*> 00000000 	nop
0+04b0 <[^>]*> 1c430000 	bltuc	v0,v1,0+04b4 <[^>]*>
[	]*4b0: R_MIPS_PC16	ext-0x4
[	]*4b0: R_MIPS_NONE	\*ABS\*-0x4
[	]*4b0: R_MIPS_NONE	\*ABS\*-0x4
0+04b4 <[^>]*> 00000000 	nop
0+04b8 <[^>]*> 1c430000 	bltuc	v0,v1,0+04bc <[^>]*>
[	]*4b8: R_MIPS_PC16	L0.*-0x20000
[	]*4b8: R_MIPS_NONE	\*ABS\*-0x20000
[	]*4b8: R_MIPS_NONE	\*ABS\*-0x20000
0+04bc <[^>]*> 00000000 	nop
0+04c0 <[^>]*> 1c430000 	bltuc	v0,v1,0+04c4 <[^>]*>
[	]*4c0: R_MIPS_PC16	L0.*\+0x1fffc
[	]*4c0: R_MIPS_NONE	\*ABS\*\+0x1fffc
[	]*4c0: R_MIPS_NONE	\*ABS\*\+0x1fffc
0+04c4 <[^>]*> 00000000 	nop
0+04c8 <[^>]*> 1c430000 	bltuc	v0,v1,0+04cc <[^>]*>
[	]*4c8: R_MIPS_PC16	.L1.*2-0x4
[	]*4c8: R_MIPS_NONE	\*ABS\*-0x4
[	]*4c8: R_MIPS_NONE	\*ABS\*-0x4
0+04cc <[^>]*> 00000000 	nop
0+04d0 <[^>]*> 1c620000 	bltuc	v1,v0,0+04d4 <[^>]*>
[	]*4d0: R_MIPS_PC16	.L1.*2-0x4
[	]*4d0: R_MIPS_NONE	\*ABS\*-0x4
[	]*4d0: R_MIPS_NONE	\*ABS\*-0x4
0+04d4 <[^>]*> 00000000 	nop
0+04d8 <[^>]*> c8000000 	bc	0+04dc <[^>]*>
[	]*4d8: R_MIPS_PC26_S2	ext-0x4
[	]*4d8: R_MIPS_NONE	\*ABS\*-0x4
[	]*4d8: R_MIPS_NONE	\*ABS\*-0x4
0+04dc <[^>]*> c8000000 	bc	0+04e0 <[^>]*>
[	]*4dc: R_MIPS_PC26_S2	L0.*-0x8000000
[	]*4dc: R_MIPS_NONE	\*ABS\*-0x8000000
[	]*4dc: R_MIPS_NONE	\*ABS\*-0x8000000
0+04e0 <[^>]*> c8000000 	bc	0+04e4 <[^>]*>
[	]*4e0: R_MIPS_PC26_S2	L0.*\+0x7fffffc
[	]*4e0: R_MIPS_NONE	\*ABS\*\+0x7fffffc
[	]*4e0: R_MIPS_NONE	\*ABS\*\+0x7fffffc
0+04e4 <[^>]*> c8000000 	bc	0+04e8 <[^>]*>
[	]*4e4: R_MIPS_PC26_S2	.L1.*2-0x4
[	]*4e4: R_MIPS_NONE	\*ABS\*-0x4
[	]*4e4: R_MIPS_NONE	\*ABS\*-0x4
0+04e8 <[^>]*> e8000000 	balc	0+04ec <[^>]*>
[	]*4e8: R_MIPS_PC26_S2	ext-0x4
[	]*4e8: R_MIPS_NONE	\*ABS\*-0x4
[	]*4e8: R_MIPS_NONE	\*ABS\*-0x4
0+04ec <[^>]*> e8000000 	balc	0+04f0 <[^>]*>
[	]*4ec: R_MIPS_PC26_S2	L0.*-0x8000000
[	]*4ec: R_MIPS_NONE	\*ABS\*-0x8000000
[	]*4ec: R_MIPS_NONE	\*ABS\*-0x8000000
0+04f0 <[^>]*> e8000000 	balc	0+04f4 <[^>]*>
[	]*4f0: R_MIPS_PC26_S2	L0.*\+0x7fffffc
[	]*4f0: R_MIPS_NONE	\*ABS\*\+0x7fffffc
[	]*4f0: R_MIPS_NONE	\*ABS\*\+0x7fffffc
0+04f4 <[^>]*> e8000000 	balc	0+04f8 <[^>]*>
[	]*4f4: R_MIPS_PC26_S2	.L1.*2-0x4
[	]*4f4: R_MIPS_NONE	\*ABS\*-0x4
[	]*4f4: R_MIPS_NONE	\*ABS\*-0x4
0+04f8 <[^>]*> d8400000 	beqzc	v0,0+04fc <[^>]*>
[	]*4f8: R_MIPS_PC21_S2	ext-0x4
[	]*4f8: R_MIPS_NONE	\*ABS\*-0x4
[	]*4f8: R_MIPS_NONE	\*ABS\*-0x4
0+04fc <[^>]*> 00000000 	nop
0+0500 <[^>]*> d8400000 	beqzc	v0,0+0504 <[^>]*>
[	]*500: R_MIPS_PC21_S2	L0.*-0x400000
[	]*500: R_MIPS_NONE	\*ABS\*-0x400000
[	]*500: R_MIPS_NONE	\*ABS\*-0x400000
0+0504 <[^>]*> 00000000 	nop
0+0508 <[^>]*> d8400000 	beqzc	v0,0+050c <[^>]*>
[	]*508: R_MIPS_PC21_S2	L0.*\+0x3ffffc
[	]*508: R_MIPS_NONE	\*ABS\*\+0x3ffffc
[	]*508: R_MIPS_NONE	\*ABS\*\+0x3ffffc
0+050c <[^>]*> 00000000 	nop
0+0510 <[^>]*> d8400000 	beqzc	v0,0+0514 <[^>]*>
[	]*510: R_MIPS_PC21_S2	.L1.*2-0x4
[	]*510: R_MIPS_NONE	\*ABS\*-0x4
[	]*510: R_MIPS_NONE	\*ABS\*-0x4
0+0514 <[^>]*> 00000000 	nop
0+0518 <[^>]*> d8038000 	jic	v1,-32768
0+051c <[^>]*> d8037fff 	jic	v1,32767
0+0520 <[^>]*> d81f0000 	jrc	ra
0+0524 <[^>]*> f8400000 	bnezc	v0,0+0528 <[^>]*>
[	]*524: R_MIPS_PC21_S2	ext-0x4
[	]*524: R_MIPS_NONE	\*ABS\*-0x4
[	]*524: R_MIPS_NONE	\*ABS\*-0x4
0+0528 <[^>]*> 00000000 	nop
0+052c <[^>]*> f8400000 	bnezc	v0,0+0530 <[^>]*>
[	]*52c: R_MIPS_PC21_S2	L0.*-0x400000
[	]*52c: R_MIPS_NONE	\*ABS\*-0x400000
[	]*52c: R_MIPS_NONE	\*ABS\*-0x400000
0+0530 <[^>]*> 00000000 	nop
0+0534 <[^>]*> f8400000 	bnezc	v0,0+0538 <[^>]*>
[	]*534: R_MIPS_PC21_S2	L0.*\+0x3ffffc
[	]*534: R_MIPS_NONE	\*ABS\*\+0x3ffffc
[	]*534: R_MIPS_NONE	\*ABS\*\+0x3ffffc
0+0538 <[^>]*> 00000000 	nop
0+053c <[^>]*> f8400000 	bnezc	v0,0+0540 <[^>]*>
[	]*53c: R_MIPS_PC21_S2	.L1.*2-0x4
[	]*53c: R_MIPS_NONE	\*ABS\*-0x4
[	]*53c: R_MIPS_NONE	\*ABS\*-0x4
0+0540 <[^>]*> 00000000 	nop
0+0544 <[^>]*> f8038000 	jialc	v1,-32768
0+0548 <[^>]*> f8037fff 	jialc	v1,32767
0+054c <[^>]*> 3c43ffff 	aui	v1,v0,0xffff
0+0550 <[^>]*> ec600000 	lapc	v1,0+0550 <[^>]*>
[	]*550: R_MIPS_PC19_S2	.L1.*2
[	]*550: R_MIPS_NONE	\*ABS\*
[	]*550: R_MIPS_NONE	\*ABS\*
0+0554 <[^>]*> ec800000 	lapc	a0,0+0554 <[^>]*>
[	]*554: R_MIPS_PC19_S2	L0.*-0x100000
[	]*554: R_MIPS_NONE	\*ABS\*-0x100000
[	]*554: R_MIPS_NONE	\*ABS\*-0x100000
0+0558 <[^>]*> ec800000 	lapc	a0,0+0558 <[^>]*>
[	]*558: R_MIPS_PC19_S2	L0.*\+0xffffc
[	]*558: R_MIPS_NONE	\*ABS\*\+0xffffc
[	]*558: R_MIPS_NONE	\*ABS\*\+0xffffc
0+055c <[^>]*> ec840000 	lapc	a0,f+ffff0055c <[^>]*>
0+0560 <[^>]*> ec83ffff 	lapc	a0,000000000010055c <[^>]*>
0+0564 <[^>]*> ec7effff 	auipc	v1,0xffff
0+0568 <[^>]*> ec7fffff 	aluipc	v1,0xffff
0+056c <[^>]*> ec880000 	lwpc	a0,0+056c <[^>]*>
[	]*56c: R_MIPS_PC19_S2	.L1.*2
[	]*56c: R_MIPS_NONE	\*ABS\*
[	]*56c: R_MIPS_NONE	\*ABS\*
0+0570 <[^>]*> ec880000 	lwpc	a0,0+0570 <[^>]*>
[	]*570: R_MIPS_PC19_S2	L0.*-0x100000
[	]*570: R_MIPS_NONE	\*ABS\*-0x100000
[	]*570: R_MIPS_NONE	\*ABS\*-0x100000
0+0574 <[^>]*> ec880000 	lwpc	a0,0+0574 <[^>]*>
[	]*574: R_MIPS_PC19_S2	L0.*\+0xffffc
[	]*574: R_MIPS_NONE	\*ABS\*\+0xffffc
[	]*574: R_MIPS_NONE	\*ABS\*\+0xffffc
0+0578 <[^>]*> ec8c0000 	lwpc	a0,f+ffff00578 <[^>]*>
0+057c <[^>]*> ec8bffff 	lwpc	a0,0000000000100578 <[^>]*>
0+0580 <[^>]*> 00000000 	nop
0+0584 <[^>]*> ec83ffff 	lapc	a0,0000000000100580 <[^>]*>
0+0588 <[^>]*> f8040000 	jalrc	a0
0+058c <[^>]*> 04100000 	nal
0+0590 <[^>]*> 00000000 	nop
0+0594 <[^>]*> 41600004 	evp
0+0598 <[^>]*> 41600024 	dvp
0+059c <[^>]*> 41620004 	evp	v0
0+05a0 <[^>]*> 41620024 	dvp	v0
0+05a4 <[^>]*> 04170000 	sigrie	0x0
0+05a8 <[^>]*> 0417ffff 	sigrie	0xffff
0+05ac <[^>]*> 7cc52076 	llwp	a1,a0,a2
0+05b0 <[^>]*> 7cc52066 	scwp	a1,a0,a2
	\.\.\.
