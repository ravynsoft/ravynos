#name: Valid v8-r
#source: armv8-ar.s
#as: -march=armv8-r
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> e320f005 	sevl
0[0-9a-f]+ <[^>]+> e1000070 	hlt	0x0000
0[0-9a-f]+ <[^>]+> e100007f 	hlt	0x000f
0[0-9a-f]+ <[^>]+> e10fff70 	hlt	0xfff0
0[0-9a-f]+ <[^>]+> e1c0fc90 	stlb	r0, \[r0\]
0[0-9a-f]+ <[^>]+> e1c1fc91 	stlb	r1, \[r1\]
0[0-9a-f]+ <[^>]+> e1cefc9e 	stlb	lr, \[lr\]
0[0-9a-f]+ <[^>]+> e1e0fc90 	stlh	r0, \[r0\]
0[0-9a-f]+ <[^>]+> e1e1fc91 	stlh	r1, \[r1\]
0[0-9a-f]+ <[^>]+> e1eefc9e 	stlh	lr, \[lr\]
0[0-9a-f]+ <[^>]+> e180fc90 	stl	r0, \[r0\]
0[0-9a-f]+ <[^>]+> e181fc91 	stl	r1, \[r1\]
0[0-9a-f]+ <[^>]+> e18efc9e 	stl	lr, \[lr\]
0[0-9a-f]+ <[^>]+> e1ce0e91 	stlexb	r0, r1, \[lr\]
0[0-9a-f]+ <[^>]+> e1c01e9e 	stlexb	r1, lr, \[r0\]
0[0-9a-f]+ <[^>]+> e1c1ee90 	stlexb	lr, r0, \[r1\]
0[0-9a-f]+ <[^>]+> e1ee0e91 	stlexh	r0, r1, \[lr\]
0[0-9a-f]+ <[^>]+> e1e01e9e 	stlexh	r1, lr, \[r0\]
0[0-9a-f]+ <[^>]+> e1e1ee90 	stlexh	lr, r0, \[r1\]
0[0-9a-f]+ <[^>]+> e18e0e91 	stlex	r0, r1, \[lr\]
0[0-9a-f]+ <[^>]+> e1801e9e 	stlex	r1, lr, \[r0\]
0[0-9a-f]+ <[^>]+> e181ee90 	stlex	lr, r0, \[r1\]
0[0-9a-f]+ <[^>]+> e1ae0e92 	stlexd	r0, r2, r3, \[lr\]
0[0-9a-f]+ <[^>]+> e1a01e9c 	stlexd	r1, ip, sp, \[r0\]
0[0-9a-f]+ <[^>]+> e1a1ee90 	stlexd	lr, r0, r1, \[r1\]
0[0-9a-f]+ <[^>]+> e1d00c9f 	ldab	r0, \[r0\]
0[0-9a-f]+ <[^>]+> e1d11c9f 	ldab	r1, \[r1\]
0[0-9a-f]+ <[^>]+> e1deec9f 	ldab	lr, \[lr\]
0[0-9a-f]+ <[^>]+> e1f00c9f 	ldah	r0, \[r0\]
0[0-9a-f]+ <[^>]+> e1f11c9f 	ldah	r1, \[r1\]
0[0-9a-f]+ <[^>]+> e1feec9f 	ldah	lr, \[lr\]
0[0-9a-f]+ <[^>]+> e1900c9f 	lda	r0, \[r0\]
0[0-9a-f]+ <[^>]+> e1911c9f 	lda	r1, \[r1\]
0[0-9a-f]+ <[^>]+> e19eec9f 	lda	lr, \[lr\]
0[0-9a-f]+ <[^>]+> e1d00e9f 	ldaexb	r0, \[r0\]
0[0-9a-f]+ <[^>]+> e1d11e9f 	ldaexb	r1, \[r1\]
0[0-9a-f]+ <[^>]+> e1deee9f 	ldaexb	lr, \[lr\]
0[0-9a-f]+ <[^>]+> e1f00e9f 	ldaexh	r0, \[r0\]
0[0-9a-f]+ <[^>]+> e1f11e9f 	ldaexh	r1, \[r1\]
0[0-9a-f]+ <[^>]+> e1feee9f 	ldaexh	lr, \[lr\]
0[0-9a-f]+ <[^>]+> e1900e9f 	ldaex	r0, \[r0\]
0[0-9a-f]+ <[^>]+> e1911e9f 	ldaex	r1, \[r1\]
0[0-9a-f]+ <[^>]+> e19eee9f 	ldaex	lr, \[lr\]
0[0-9a-f]+ <[^>]+> e1b00e9f 	ldaexd	r0, r1, \[r0\]
0[0-9a-f]+ <[^>]+> e1b12e9f 	ldaexd	r2, r3, \[r1\]
0[0-9a-f]+ <[^>]+> e1bece9f 	ldaexd	ip, sp, \[lr\]
0[0-9a-f]+ <[^>]+> bf50      	sevl
0[0-9a-f]+ <[^>]+> bf50      	sevl
0[0-9a-f]+ <[^>]+> f3af 8005 	sevl.w
0[0-9a-f]+ <[^>]+> f78f 8001 	dcps1
0[0-9a-f]+ <[^>]+> f78f 8002 	dcps2
0[0-9a-f]+ <[^>]+> f78f 8003 	dcps3
0[0-9a-f]+ <[^>]+> ba80      	hlt	0x0000
0[0-9a-f]+ <[^>]+> babf      	hlt	0x003f
0[0-9a-f]+ <[^>]+> e8c0 0f8f 	stlb	r0, \[r0\]
0[0-9a-f]+ <[^>]+> e8c1 1f8f 	stlb	r1, \[r1\]
0[0-9a-f]+ <[^>]+> e8ce ef8f 	stlb	lr, \[lr\]
0[0-9a-f]+ <[^>]+> e8c0 0f9f 	stlh	r0, \[r0\]
0[0-9a-f]+ <[^>]+> e8c1 1f9f 	stlh	r1, \[r1\]
0[0-9a-f]+ <[^>]+> e8ce ef9f 	stlh	lr, \[lr\]
0[0-9a-f]+ <[^>]+> e8c0 0faf 	stl	r0, \[r0\]
0[0-9a-f]+ <[^>]+> e8c1 1faf 	stl	r1, \[r1\]
0[0-9a-f]+ <[^>]+> e8ce efaf 	stl	lr, \[lr\]
0[0-9a-f]+ <[^>]+> e8ce 1fc0 	stlexb	r0, r1, \[lr\]
0[0-9a-f]+ <[^>]+> e8c0 efc1 	stlexb	r1, lr, \[r0\]
0[0-9a-f]+ <[^>]+> e8c1 0fce 	stlexb	lr, r0, \[r1\]
0[0-9a-f]+ <[^>]+> e8ce 1fd0 	stlexh	r0, r1, \[lr\]
0[0-9a-f]+ <[^>]+> e8c0 efd1 	stlexh	r1, lr, \[r0\]
0[0-9a-f]+ <[^>]+> e8c1 0fde 	stlexh	lr, r0, \[r1\]
0[0-9a-f]+ <[^>]+> e8ce 1fe0 	stlex	r0, r1, \[lr\]
0[0-9a-f]+ <[^>]+> e8c0 efe1 	stlex	r1, lr, \[r0\]
0[0-9a-f]+ <[^>]+> e8c1 0fee 	stlex	lr, r0, \[r1\]
0[0-9a-f]+ <[^>]+> e8ce 11f0 	stlexd	r0, r1, r1, \[lr\]
0[0-9a-f]+ <[^>]+> e8c0 eef1 	stlexd	r1, lr, lr, \[r0\]
0[0-9a-f]+ <[^>]+> e8c1 00fe 	stlexd	lr, r0, r0, \[r1\]
0[0-9a-f]+ <[^>]+> e8d0 0f8f 	ldab	r0, \[r0\]
0[0-9a-f]+ <[^>]+> e8d1 1f8f 	ldab	r1, \[r1\]
0[0-9a-f]+ <[^>]+> e8de ef8f 	ldab	lr, \[lr\]
0[0-9a-f]+ <[^>]+> e8d0 0f9f 	ldah	r0, \[r0\]
0[0-9a-f]+ <[^>]+> e8d1 1f9f 	ldah	r1, \[r1\]
0[0-9a-f]+ <[^>]+> e8de ef9f 	ldah	lr, \[lr\]
0[0-9a-f]+ <[^>]+> e8d0 0faf 	lda	r0, \[r0\]
0[0-9a-f]+ <[^>]+> e8d1 1faf 	lda	r1, \[r1\]
0[0-9a-f]+ <[^>]+> e8de efaf 	lda	lr, \[lr\]
0[0-9a-f]+ <[^>]+> e8d0 0fcf 	ldaexb	r0, \[r0\]
0[0-9a-f]+ <[^>]+> e8d1 1fcf 	ldaexb	r1, \[r1\]
0[0-9a-f]+ <[^>]+> e8de efcf 	ldaexb	lr, \[lr\]
0[0-9a-f]+ <[^>]+> e8d0 0fdf 	ldaexh	r0, \[r0\]
0[0-9a-f]+ <[^>]+> e8d1 1fdf 	ldaexh	r1, \[r1\]
0[0-9a-f]+ <[^>]+> e8de efdf 	ldaexh	lr, \[lr\]
0[0-9a-f]+ <[^>]+> e8d0 0fef 	ldaex	r0, \[r0\]
0[0-9a-f]+ <[^>]+> e8d1 1fef 	ldaex	r1, \[r1\]
0[0-9a-f]+ <[^>]+> e8de efef 	ldaex	lr, \[lr\]
0[0-9a-f]+ <[^>]+> e8d0 01ff 	ldaexd	r0, r1, \[r0\]
0[0-9a-f]+ <[^>]+> e8d1 1eff 	ldaexd	r1, lr, \[r1\]
0[0-9a-f]+ <[^>]+> e8de e0ff 	ldaexd	lr, r0, \[lr\]
