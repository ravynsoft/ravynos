        .section .text.foo,"ax",@progbits
        .type foo, @function
foo:
        .global foo
        movl ifunc@GOTPCREL(%rip), %eax
        movl ifunc(%rip), %eax
	call ifunc@PLT
	call ifunc
        movl xxx(%rip), %eax
        ret

        .section .text.bar,"ax",@progbits
        .type bar, @function
bar:
        .global bar
        ret

        .section .text.ifunc,"ax",@progbits
        .type ifunc, @gnu_indirect_function
ifunc:
        ret

        .section .data.foo,"aw",@progbits
xxx:
	.quad ifunc 
