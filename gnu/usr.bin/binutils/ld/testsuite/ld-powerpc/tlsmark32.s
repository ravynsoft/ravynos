	.section ".tdata","awT",@progbits
x:	.int 1

	.text
	.global _start
_start:
	b .L2

.L1:
	bl __tls_get_addr(x@tlsgd)
	lwz 4,0(3)
	addi 3,31,x@got@tlsld
	b .L3
.L2:
	addi 3,31,x@got@tlsgd
	b .L1
.L3:
	bl __tls_get_addr(x@tlsld)
	lwz 4,x@dtprel(3)
