        .text
	.arch armv4t
	.syntax unified
	.thumb
one:
	bl	foo2  @ bl foo2 with reloc.
	beq	foo   @ beq foo with reloc.
	b	foo   @ branch foo with reloc.
 	bl      fooundefarm
	bl      fooundefthumb
	.thumb
        .type foo, %function
        .thumb_func
foo:
        nop
	nop
fooundefthumb:
	nop
        .type foo2, %function
	.arm
	.align  2
foo2:
        nop
fooundefarm:
	nop
