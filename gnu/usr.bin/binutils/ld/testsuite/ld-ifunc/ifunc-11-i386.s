        .section .text.foo,"ax",@progbits
        .type foo, @function
foo:
        .global foo
        movl ifunc@GOT(%ecx), %eax
        movl ifunc@GOTOFF(%ecx), %eax
	call ifunc@PLT
	call ifunc
        movl xxx@GOT(%ecx), %eax
        movl xxx, %eax
        ret

        .section .text.bar,"ax",@progbits
        .type bar, @function
bar:
        .global bar
        ret

        .section .text.ifunc,"ax",@progbits
        .type ifunc, @gnu_indirect_function
	.global ifunc
ifunc:
        ret

        .section .data.foo,"aw",@progbits
xxx:
	.long ifunc 
