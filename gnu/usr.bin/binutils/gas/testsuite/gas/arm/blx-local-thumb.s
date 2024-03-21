        .text
	.arch armv5t
	.syntax unified
	.thumb
one:
        blx	foo   @ bl foo
	blx     foo2  @ blx foo2
	bl	foo   @ bl foo
	bl	foo2  @ blx foo2
	blx	fooundefarm
	bl      fooundefarm
	blx     fooundefthumb
	bl      fooundefthumb
	.thumb
        .type foo, %function
        .thumb_func
foo:
	b  one	@no relocs
	b  foo2	@ THUMB_PCREL_JUMP
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
