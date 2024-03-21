foo:
	lwi $r0, [$r1 + (1 << 2)]
	lhi $r0, [$r1 + (1 << 1)]
	lhsi $r0, [$r1 + (-1 << 1)]
	lbi $r0, [$r1 + 1]
	lbsi $r0, [$r1 + (-1)]
	swi $r0, [$r1 + (1 << 2)]
	shi $r0, [$r1 + (1 << 1)]
	sbi $r0, [$r1 + 1]
	lwi.bi $r0, [$r1], (1 << 2)
	lhi.bi $r0, [$r1], (1 << 1)
	lhsi.bi $r0, [$r1], (-1 << 1)
	lbi.bi $r0, [$r1], 1
	lbsi.bi $r0, [$r1], -1
	swi.bi $r0, [$r1], (1 << 2)
	shi.bi $r0, [$r1], (1 << 1)
	sbi.bi $r0, [$r1], 1
