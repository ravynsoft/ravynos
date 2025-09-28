	.text
	.arm
	.globl foo
	.type foo, %function
foo:
	ldr	r0, 1f
2:
	blx	lib_gd2(tlscall) 
	nop

1:
	.word	lib_gd2(tlsdesc) + (. - 2b)

	.thumb
	.globl bar
	.type bar, %function
bar:
	ldr	r0,1f
2:
	blx	lib_gd2(tlscall) 
	nop

	.p2align 2
1:
	.word	lib_gd2(tlsdesc) + (. - 2b + 1)

	.globl _start
_start:
	
	.section .tdata,"awT"
	.global lib_gd2
lib_gd2:
	.space	4

