	.syntax unified
	.text
	.arch armv8-a

	.arm
foo:
	sevl
	hlt 0x0
	hlt 0xf
	hlt 0xfff0
	stlb r0, [r0]
	stlb r1, [r1]
	stlb r14, [r14]
	stlh r0, [r0]
	stlh r1, [r1]
	stlh r14, [r14]
	stl r0, [r0]
	stl r1, [r1]
	stl r14, [r14]
	stlexb r0, r1, [r14]
	stlexb r1, r14, [r0]
	stlexb r14, r0, [r1]
	stlexh r0, r1, [r14]
	stlexh r1, r14, [r0]
	stlexh r14, r0, [r1]
	stlex r0, r1, [r14]
	stlex r1, r14, [r0]
	stlex r14, r0, [r1]
	stlexd r0, r2, r3, [r14]
	stlexd r1, r12, r13, [r0]
	stlexd r14, r0, r1, [r1]
	ldab r0, [r0]
	ldab r1, [r1]
	ldab r14, [r14]
	ldah r0, [r0]
	ldah r1, [r1]
	ldah r14, [r14]
	lda r0, [r0]
	lda r1, [r1]
	lda r14, [r14]
	ldaexb r0, [r0]
	ldaexb r1, [r1]
	ldaexb r14, [r14]
	ldaexh r0, [r0]
	ldaexh r1, [r1]
	ldaexh r14, [r14]
	ldaex r0, [r0]
	ldaex r1, [r1]
	ldaex r14, [r14]
	ldaexd r0, r1, [r0]
	ldaexd r2, r3, [r1]
	ldaexd r12, r13, [r14]

	.thumb
	.thumb_func
bar:
	sevl
	sevl.n
	sevl.w
	dcps1
	dcps2
	dcps3
	hlt 0
	hlt 63
	stlb r0, [r0]
	stlb r1, [r1]
	stlb r14, [r14]
	stlh r0, [r0]
	stlh r1, [r1]
	stlh r14, [r14]
	stl r0, [r0]
	stl r1, [r1]
	stl r14, [r14]
	stlexb r0, r1, [r14]
	stlexb r1, r14, [r0]
	stlexb r14, r0, [r1]
	stlexh r0, r1, [r14]
	stlexh r1, r14, [r0]
	stlexh r14, r0, [r1]
	stlex r0, r1, [r14]
	stlex r1, r14, [r0]
	stlex r14, r0, [r1]
	stlexd r0, r1, r1, [r14]
	stlexd r1, r14, r14, [r0]
	stlexd r14, r0, r0, [r1]
	ldab r0, [r0]
	ldab r1, [r1]
	ldab r14, [r14]
	ldah r0, [r0]
	ldah r1, [r1]
	ldah r14, [r14]
	lda r0, [r0]
	lda r1, [r1]
	lda r14, [r14]
	ldaexb r0, [r0]
	ldaexb r1, [r1]
	ldaexb r14, [r14]
	ldaexh r0, [r0]
	ldaexh r1, [r1]
	ldaexh r14, [r14]
	ldaex r0, [r0]
	ldaex r1, [r1]
	ldaex r14, [r14]
	ldaexd r0, r1, [r0]
	ldaexd r1, r14, [r1]
	ldaexd r14, r0, [r14]
