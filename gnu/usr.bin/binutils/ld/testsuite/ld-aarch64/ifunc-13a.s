	.text
        .type foo, @function
	.global foo
foo:
	adrp    x0, xxx
        add     x0, x0, :lo12:xxx
        ret

	.data
xxx:
	.quad ifunc 
