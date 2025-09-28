#source: copro-arm_v2plus-thumb_v6t2plus.s
#objdump: -dr --prefix-addresses --show-raw-insn --disassembler-options=force-thumb
#name: ARMv6T2 Thumb CoProcessor Instructions (1)
#as: -march=armv6t2 -mthumb -mimplicit-it=always -EL

# Test the standard ARM co-processor instructions:

.*: +file format .*arm.*

Disassembly of section .text:
0+000 <[^>]*> ee42 1103 	dvfs	f1, f2, f3
0+004 <[^>]*> [^ ]*      	it	eq
0+006 <[^>]*> ee34 14a5 	cfadddeq	mvd1, mvd4, mvd5
0+00a <[^>]*> ed93 9500 	cfldr32	mvfx9, \[r3\]
0+00e <[^>]*> edd1 e108 	ldfp	f6, \[r1, #32\]
0+012 <[^>]*> [^ ]*      	ite	mi
0+014 <[^>]*> edb2 00ff 	ldcmi	0, cr0, \[r2, #1020\]!.*
0+018 <[^>]*> ecf3 1710 	ldclpl	7, cr1, \[r3\], #64.*
0+01c <[^>]*> ed9f 8000 	ldc	0, cr8, \[pc]	@ .* <foo>
0+020 <[^>]*> ed83 0500 	cfstr32	mvfx0, \[r3\]
0+024 <[^>]*> edc0 f302 	stcl	3, cr15, \[r0, #8\]
0+028 <[^>]*> [^ ]*      	it	eq
0+02a <[^>]*> eda2 c419 	cfstrseq	mvf12, \[r2, #100\]!.*
0+02e <[^>]*> [^ ]*      	it	cc
0+030 <[^>]*> eca4 860c 	stccc	6, cr8, \[r4\], #48.*
0+034 <[^>]*> ed8f 7100 	stfs	f7, \[pc\]	@ .* <bar>
0+038 <[^>]*> ee71 5212 	mrc	2, 3, r5, cr1, cr2, \{0\}
0+03c <[^>]*> [^ ]*      	it	ge
0+03e <[^>]*> eeb1 f4f2 	mrcge	4, 5, APSR_nzcv, cr1, cr2, \{7\}
0+042 <[^>]*> ee21 5711 	mcr	7, 1, r5, cr1, cr1, \{0\}
0+046 <[^>]*> [^ ]*      	it	lt
0+048 <[^>]*> ee22 8519 	mcrlt	5, 1, r8, cr2, cr9, \{0\}
0+04c <[^>]*> ec90 7300 	ldc	3, cr7, \[r0\], \{0\}
0+050 <[^>]*> ec81 6e01 	stc	14, cr6, \[r1\], \{1\}
0+054 <[^>]*> ecd4 3704 	ldcl	7, cr3, \[r4\], \{4\}
0+058 <[^>]*> ecc5 2805 	stcl	8, cr2, \[r5\], \{5\}
0+05c <[^>]*> ecd8 8cff 	ldcl	12, cr8, \[r8\], \{255\}.*
0+060 <[^>]*> ecc9 9cfe 	stcl	12, cr9, \[r9\], \{254\}.*
0+064 <[^>]*> bf00      	nop
0+066 <[^>]*> bf00      	nop
0+068 <[^>]*> [^ ]*      	it	ge
0+06a <[^>]*> eeb1 f4f2 	mrcge	4, 5, APSR_nzcv, cr1, cr2, \{7\}
#...
