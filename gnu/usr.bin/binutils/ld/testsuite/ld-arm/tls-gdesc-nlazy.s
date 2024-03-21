	.text
	.globl foo
	.type foo, %function
foo:
	nop
.L2:
	blx	lib_gd2(tlscall) 
	mov	pc, lr

.Lpool:
	.word	lib_gd2(tlsdesc) + (. - .L2)

	.section .tdata,"awT"
	.global lib_gd2
lib_gd2:
	.space	4

