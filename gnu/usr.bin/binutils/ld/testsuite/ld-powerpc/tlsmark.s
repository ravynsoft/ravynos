	.section ".tdata","awT",@progbits
x:	.int 1
y:	.int 2

	.section ".toc","aw",@progbits
	.p2align 3
.LC0:
	.quad y@dtpmod
	.quad y@dtprel
.LC1:
	.quad y@dtpmod
	.quad 0

	.text
	.global _start
_start:
	b .L2

.L1:
	bl __tls_get_addr(x@tlsgd)
	nop
	ld 4,0(3)
	addi 3,2,x@got@tlsld
	b .L3
.L2:
	addi 3,2,x@got@tlsgd
	b .L1
.L3:
	bl __tls_get_addr(x@tlsld)
	nop
	ld 4,x@dtprel(3)

	addi 3,2,.LC0@toc
	b .L5
.L4:
	addi 3,2,.LC1@toc
	b .L6
.L5:
	bl .__tls_get_addr(.LC0@tlsgd)
	nop
	ld 5,0(3)
	b .L4
.L6:
	bl .__tls_get_addr(.LC1@tlsld)
	nop
	ld 5,y@dtprel(3)
