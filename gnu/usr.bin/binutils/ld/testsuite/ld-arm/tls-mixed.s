	.text
	.globl foo
	.type foo, %function
foo:
	nop
.L1:
	nop	
.L2:
	bl	lib_gd2(tlscall) 
	mov	pc, lr

.Lpool:
	.word	lib_gd(tlsgd)	+  (. - .L1 - 8)
.Lpool2:
	.word	lib_gd2(tlsdesc) + (. - .L2)
	.word	lib_gd2(tlsgd)	+  (. - .L2 - 8)

	.section .tdata,"awT"
	.global lib_gd
lib_gd:
	.space	4 
	.global lib_gd2
lib_gd2:
	.space	4

