#name: ARM V8-M mainline instructions
#source: archv8m.s
#as: -march=armv8-m.main
#objdump: -dr --prefix-addresses --show-raw-insn -M force-thumb

.*: +file format .*arm.*

Disassembly of section .text:
0+.* <[^>]*> 47a0      	blx	r4
0+.* <[^>]*> 47c8      	blx	r9
0+.* <[^>]*> 4720      	bx	r4
0+.* <[^>]*> 4748      	bx	r9
0+.* <[^>]*> e841 f000 	tt	r0, r1
0+.* <[^>]*> e849 f800 	tt	r8, r9
0+.* <[^>]*> e841 f040 	ttt	r0, r1
0+.* <[^>]*> e849 f840 	ttt	r8, r9
0+.* <[^>]*> f24f 1023 	movw	r0, #61731	@ 0xf123
0+.* <[^>]*> f24f 1823 	movw	r8, #61731	@ 0xf123
0+.* <[^>]*> f24f 1823 	movw	r8, #61731	@ 0xf123
0+.* <[^>]*> f24f 1823 	movw	r8, #61731	@ 0xf123
0+.* <[^>]*> f2cf 1023 	movt	r0, #61731	@ 0xf123
0+.* <[^>]*> f2cf 1823 	movt	r8, #61731	@ 0xf123
0+.* <[^>]*> b154      	cbz	r4, 0+.* <[^>]*>
0+.* <[^>]*> b94c      	cbnz	r4, 0+.* <[^>]*>
0+.* <[^>]*> f000 b808 	b.w	0+.* <[^>]*>
0+.* <[^>]*> fb91 f0f2 	sdiv	r0, r1, r2
0+.* <[^>]*> fb99 f8fa 	sdiv	r8, r9, sl
0+.* <[^>]*> fbb1 f0f2 	udiv	r0, r1, r2
0+.* <[^>]*> fbb9 f8fa 	udiv	r8, r9, sl
0+.* <[^>]*> 4408      	add	r0, r1
0+.* <[^>]*> f3bf 8f2f 	clrex
0+.* <[^>]*> e851 0f01 	ldrex	r0, \[r1, #4\]
0+.* <[^>]*> e8d1 0f4f 	ldrexb	r0, \[r1\]
0+.* <[^>]*> e8d1 0f5f 	ldrexh	r0, \[r1\]
0+.* <[^>]*> e842 1001 	strex	r0, r1, \[r2, #4\]
0+.* <[^>]*> e8c2 1f40 	strexb	r0, r1, \[r2\]
0+.* <[^>]*> e8c2 1f50 	strexh	r0, r1, \[r2\]
0+.* <[^>]*> e8d1 0faf 	lda	r0, \[r1\]
0+.* <[^>]*> e8d1 0f8f 	ldab	r0, \[r1\]
0+.* <[^>]*> e8d1 0f9f 	ldah	r0, \[r1\]
0+.* <[^>]*> e8c1 0faf 	stl	r0, \[r1\]
0+.* <[^>]*> e8c1 0f8f 	stlb	r0, \[r1\]
0+.* <[^>]*> e8c1 0f9f 	stlh	r0, \[r1\]
0+.* <[^>]*> e8d1 0fef 	ldaex	r0, \[r1\]
0+.* <[^>]*> e8d1 0fcf 	ldaexb	r0, \[r1\]
0+.* <[^>]*> e8d1 0fdf 	ldaexh	r0, \[r1\]
0+.* <[^>]*> e8c2 1fe0 	stlex	r0, r1, \[r2\]
0+.* <[^>]*> e8c2 1fc0 	stlexb	r0, r1, \[r2\]
0+.* <[^>]*> e8c2 1fd0 	stlexh	r0, r1, \[r2\]
#...
