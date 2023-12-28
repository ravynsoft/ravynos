	.data
	.globl _start
_start:
	.dc.a	__start__foo
	.dc.a	bar
	.section _foo,"aw",%progbits
foo:
	.ascii "This is foo"
