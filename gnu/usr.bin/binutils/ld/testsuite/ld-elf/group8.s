	.section	.text.foo,"axG",%progbits,foo,comdat
	.globl foo
	.type	foo,%function
foo:
	.byte 0
	.section	.text.bar,"axG",%progbits,bar,comdat
	.globl bar
	.type	bar,%function
bar:
	.byte 0
