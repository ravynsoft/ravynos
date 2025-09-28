	.section	.text.foo,"axG",%progbits,foo,comdat
	.globl foo
	.type	foo,%function
foo:
	.byte 0
	.section	.data.foo,"axG",%progbits,foo,comdat
	.globl foo.data
	.type	foo.data,%object
foo.data:
	.byte 0
	.section	.text.bar,"axG",%progbits,bar,comdat
	.globl bar
	.type	bar,%function
bar:
	.long foo.data
