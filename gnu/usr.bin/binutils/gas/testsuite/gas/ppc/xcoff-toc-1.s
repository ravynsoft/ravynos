	.csect _rw_[RW],4
	.toc

	.csect .text[PR]
	.align 2
	.lglobl .foo
	.csect foo[DS]
foo:
	.long .foo, TOC[tc0], 0
	.csect .text[PR]
.foo:
	lwz 1,LC..72(2)
	blr
	.align 2
	.toc
LC..72:
	.tc data[TC],data
	.csect _rw_[RW],4
	.align 2
data:
	.space 0x10000
