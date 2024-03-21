	.cpu em4
	.text
	.align	4
test:
	jli_s	__jli.foo
	jli_s	__jli.bar

	.align	4
foo:
	add	r0,r0,r0

	.align 	4
bar:
	add	r0,r1,r2

	.section	.jlitab,"axG",%progbits,jli_group,comdat
        .align  4
__jli.foo:
        .weak   __jli.foo
        b       @foo
        .align  4
__jli.bar:
        .weak   __jli.bar
        b       @bar
