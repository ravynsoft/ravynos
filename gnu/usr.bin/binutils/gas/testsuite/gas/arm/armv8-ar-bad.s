	.syntax unified
	.text

	// SWP
	.arm
	swp r0, r1, [r2]

	// deprecated MCRs
	mcr p15, 0, r0, c7, c5, 4
	mcr p15, 0, r1, c7, c10, 4
	mcr p15, 0, r2, c7, c10, 5
	mrc p14, 6, r1, c0, c0, 0
	mrc p14, 6, r0, c1, c0, 0

	// deprecated SETEND
	setend be

	.thumb
	setend le

	// HLT A32
	.arm
	hlt 0x10000
	hltne 0x1

	// HLT T32
	.thumb
	hlt 64
	it ne
	hltne 0

	// STL A32
	.arm
	stlb pc, [r0]
	stlb r0, [pc]
	stlh pc, [r0]
	stlh r0, [pc]
	stl pc, [r0]
	stl r0, [pc]
	stlexb r1, pc, [r0]
	stlexb r1, r0, [pc]
	stlexb pc, r0, [r1]
	stlexb r0, r0, [r1]
	stlexb r0, r1, [r0]
	stlexh r1, pc, [r0]
	stlexh r1, r0, [pc]
	stlexh pc, r0, [r1]
	stlexh r0, r0, [r1]
	stlexh r0, r1, [r0]
	stlex r1, pc, [r0]
	stlex r1, r0, [pc]
	stlex pc, r0, [r1]
	stlex r0, r0, [r1]
	stlex r0, r1, [r0]
	stlexd r1, lr, [r0]
	stlexd r1, r0, [pc]
	stlexd pc, r0, [r1]
	stlexd r0, r0, [r1]
	stlexd r0, r2, [r0]
	stlexd r0, r1, [r2]

	// STL T32
	.thumb
	stlb pc, [r0]
	stlb r0, [pc]
	stlh pc, [r0]
	stlh r0, [pc]
	stl pc, [r0]
	stl r0, [pc]
	stlexb r1, pc, [r0]
	stlexb r1, r0, [pc]
	stlexb pc, r0, [r1]
	stlexb r0, r0, [r1]
	stlexb r0, r1, [r0]
	stlexh r1, pc, [r0]
	stlexh r1, r0, [pc]
	stlexh pc, r0, [r1]
	stlexh r0, r0, [r1]
	stlexh r0, r1, [r0]
	stlex r1, pc, [r0]
	stlex r1, r0, [pc]
	stlex pc, r0, [r1]
	stlex r0, r0, [r1]
	stlex r0, r1, [r0]
	stlexd r1, lr, [r0]
	stlexd r1, r0, [pc]
	stlexd pc, r0, [r1]
	stlexd r0, r0, [r1]
	stlexd r0, r2, [r0]
	stlexd r0, r1, [r2]

	// LDA A32
	.arm
	ldab pc, [r0]
	ldab r0, [pc]
	ldah pc, [r0]
	ldah r0, [pc]
	lda pc, [r0]
	lda r0, [pc]
	ldaexb pc, [r0]
	ldaexb r0, [pc]
	ldaexh pc, [r0]
	ldaexh r0, [pc]
	ldaex pc, [r0]
	ldaex r0, [pc]
	ldaexd lr, [r0]
	ldaexd r0, [pc]
	ldaexd r1, [r2]

	// LDA T32
	.thumb
	ldab pc, [r0]
	ldab r0, [pc]
	ldah pc, [r0]
	ldah r0, [pc]
	lda pc, [r0]
	lda r0, [pc]
	ldaexb pc, [r0]
	ldaexb r0, [pc]
	ldaexh pc, [r0]
	ldaexh r0, [pc]
	ldaex pc, [r0]
	ldaex r0, [pc]
	ldaexd r0, pc, [r0]
	ldaexd pc, r0, [r0]
	ldaexd r1, r0, [pc]
