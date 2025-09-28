	.text
	.arm
	.globl foo
	.type foo, %function
foo:
	nop
3:	ldr	r0,1f
2:	bl	lib_gd2(tlscall) 
	nop

1:
	.word	lib_gd2(tlsdesc) + (. - 2b)
	.word	lib_gd2(gottpoff) + (. - 3b - 8)

	.thumb
	.globl bar
	.type bar, %function
bar:
3:	ldr	r0,1f
2:	blx	lib_gd2(tlscall) 
	nop

	.p2align 2
1:
	.word	lib_gd2(tlsdesc) + (. - 2b + 1)
	.word	lib_gd2(gottpoff) + (. - 3b - 4)

	.globl _start
_start:

	.section .tdata,"awT"
	.global lib_gd2
lib_gd2:
	.space	4

