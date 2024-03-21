#source: copro-arm_v2plus-thumb_v6t2plus.s
#objdump: -dr --prefix-addresses --show-raw-insn
#name: ARMv2 ARM CoProcessor Instructions
#as: -march=armv2 -EL

# Test the standard ARM co-processor instructions:

.*: +file format .*arm.*

Disassembly of section .text:
0+000 <[^>]*> ee421103 	dvfs	f1, f2, f3
0+004 <[^>]*> 0e3414a5 	cfadddeq	mvd1, mvd4, mvd5
0+008 <[^>]*> ed939500 	cfldr32	mvfx9, \[r3\]
0+00c <[^>]*> edd1e108 	ldfp	f6, \[r1, #32\]
0+010 <[^>]*> 4db200ff 	ldcmi	0, cr0, \[r2, #1020\]!.*
0+014 <[^>]*> 5cf31710 	ldclpl	7, cr1, \[r3\], #64.*
0+018 <[^>]*> ed1f8001 	ldc	0, cr8, \[pc, #-4\]	@ .* <foo>
0+01c <[^>]*> ed830500 	cfstr32	mvfx0, \[r3\]
0+020 <[^>]*> edc0f302 	stcl	3, cr15, \[r0, #8\]
0+024 <[^>]*> 0da2c419 	cfstrseq	mvf12, \[r2, #100\]!.*
0+028 <[^>]*> 3ca4860c 	stccc	6, cr8, \[r4\], #48.*
0+02c <[^>]*> ed0f7101 	stfs	f7, \[pc, #-4\]	@ .* <bar>
0+030 <[^>]*> ee715212 	mrc	2, 3, r5, cr1, cr2, \{0\}
0+034 <[^>]*> aeb1f4f2 	mrcge	4, 5, APSR_nzcv, cr1, cr2, \{7\}
0+038 <[^>]*> ee215711 	mcr	7, 1, r5, cr1, cr1, \{0\}
0+03c <[^>]*> be228519 	mcrlt	5, 1, r8, cr2, cr9, \{0\}
0+040 <[^>]*> ec907300 	ldc	3, cr7, \[r0\], \{0\}
0+044 <[^>]*> ec816e01 	stc	14, cr6, \[r1\], \{1\}
0+048 <[^>]*> ecd43704 	ldcl	7, cr3, \[r4\], \{4\}
0+04c <[^>]*> ecc52805 	stcl	8, cr2, \[r5\], \{5\}
0+050 <[^>]*> ecd88cff 	ldcl	12, cr8, \[r8\], \{255\}.*
0+054 <[^>]*> ecc99cfe 	stcl	12, cr9, \[r9\], \{254\}.*
0+058 <[^>]*> e1a00000 	nop			@ \(mov r0, r0\)
0+05c <[^>]*> e1a00000 	nop			@ \(mov r0, r0\)
0+060 <[^>]*> aeb1f4f2 	mrcge	4, 5, APSR_nzcv, cr1, cr2, \{7\}
