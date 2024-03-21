	.text
        .type foo, @function
	.global foo
foo:
	movl xxx@GOT(%ebx), %eax
        ret

	.data
xxx:
	.long ifunc 
