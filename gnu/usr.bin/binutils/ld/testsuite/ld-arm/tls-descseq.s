	.text
	.arm
	.globl foo
	.type foo, %function
foo:
	ldr	r0, 1f
2:
.tlsdescseq lib_gd2
	add	r0, pc, r0
.tlsdescseq lib_gd2
	ldr	r1, [r0,#4]
.tlsdescseq lib_gd2
	blx	r1
	nop

1:
	.word	lib_gd2(tlsdesc) + (. - 2b)

	.thumb
	.globl bar
	.type bar, %function
bar:
	ldr	r0, 1f
2:
.tlsdescseq lib_gd2
	add	r0, pc
.tlsdescseq lib_gd2
	ldr	r1, [r0,#4]
.tlsdescseq lib_gd2
	blx	r1
	nop

	.p2align 2
1:
	.word	lib_gd2(tlsdesc) + (. - 2b + 1)

	.section .tdata,"awT"
	.global lib_gd2
lib_gd2:
	.space	4

