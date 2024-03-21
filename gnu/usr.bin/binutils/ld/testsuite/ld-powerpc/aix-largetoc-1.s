	.globl a
	.csect .data[RW]
a:
	.long	1
	.toc
	.tc a[TE],a
	.tc b[TE],a

	.globl foo
	.globl .foo
	.csect foo[DS],3
foo:
	.if size == 32
	.long .foo, TOC[tc0], 0
	.else
	.llong .foo, TOC[tc0], 0
	.endif

	.csect .text[PR]
.foo:
	addis 9,a[TE]@u(2)
	la 9,a[TE]@l(9)

	addis 9,b[TE]@u(2)
	la 9,b[TE]@l(9)
