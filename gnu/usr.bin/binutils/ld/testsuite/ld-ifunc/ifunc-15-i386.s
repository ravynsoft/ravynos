	.text
        .type foo, @function
	.global foo
foo:
	movl ifunc@GOT(%ebx), %eax
        ret
        .type ifunc, @gnu_indirect_function
	.globl ifunc
ifunc:
        ret
