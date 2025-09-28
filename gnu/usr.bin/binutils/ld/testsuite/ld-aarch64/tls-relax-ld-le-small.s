	.cpu generic+fp+simd
	.section	.text.startup,"ax",%progbits
	.align	2
	.p2align 3,,7
	.global	main
	.type	main, %function
main:
	add	x29, sp, 0
	adrp	x0, :tlsldm:global_a0
	add	x0, x0, #:tlsldm_lo12_nc:global_a0
	bl	__tls_get_addr
	nop
	add	x1, x0, #:dtprel_hi12:global_a0, lsl #12
	add	x1, x1, #:dtprel_lo12_nc:global_a0
	adrp	x0, .LC0
	ret
	.size	main, .-main
	.section	.rodata.str1.8,"aMS",%progbits,1
	.align	3
.LC0:
	.string	"Hello world %d\n"
	.section	.tdata,"awT",%progbits
	.align	2
	.type	global_a0, %object
	.size	global_a0, 4
global_a0:
	.word	16
