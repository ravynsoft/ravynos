	.globl _start
_start:
 .ifdef UNDERSCORE
	.weak	___start__foo
	.dc.a	___start__foo
 .else
	.weak	__start__foo
	.dc.a	__start__foo
 .endif
	.section	_foo,"aw",%progbits
foo:
	.long	1
