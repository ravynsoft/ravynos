@ we can relax local and non-weak globals for non-shared links

	.text
	.arm

	.p2align 2
foo:
@tlscall global, manually relaxed to IE
	ldr	r0, 1f
2:	ldr	r0, [pc, r0]
	nop
	.p2align 2
1:	.word	gd1(gottpoff) + (. - 2b - 8)
	
	.p2align 2
@tlscall global, should relax to IE
	ldr	r0, 1f
2:	blx	gd1(tlscall)
	nop
	.p2align 2
1:	.word	gd1(tlsdesc) + (. - 2b)

	.p2align 2
@tlscall local, manually relaxed to LE
	ldr	r0, 1f
	nop
	nop
	.p2align 2
1:	.word	ld1(tpoff)

	.p2align 2
@tlscall local, should relax to LE
	ldr	r0, 1f
2:	blx	ld1(tlscall)
	nop
	.p2align 2
1:	.word	ld1(tlsdesc) + (. - 2b)

	.p2align 2
@open coded global, manually relaxed to IE
	ldr	r0, 1f
2:
	add	r0, pc, r0
	ldr	r1, [r0]
	mov	r0, r1
	nop
	.p2align 2
1:	.word	gd1(gottpoff) + (. - 2b - 8)

	.p2align 2
@open coded global, should relax to IE
	ldr	r0, 1f
2:
.tlsdescseq gd1
	add	r0, pc, r0
.tlsdescseq gd1
	ldr	r1, [r0,#4]
.tlsdescseq gd1
	blx	r1
	nop
	.p2align 2
1:	.word	gd1(tlsdesc) + (. - 2b)

	.p2align 2
@open coded local, manually relaxed to LE
	ldr	r0, 1f
2:
	nop
	nop
	nop
	nop
	.p2align 2
1:	.word	ld1(tpoff)
	
	.p2align 2
@open coded local, should relax to LE
	ldr	r0, 1f
2:
.tlsdescseq ld1
	add	r0, pc, r0
.tlsdescseq ld1
	ldr	r1, [r0,#4]
.tlsdescseq ld1
	blx	r1
	nop
	.p2align 2
1:	.word	ld1(tlsdesc) + (. - 2b)


	.thumb
	.p2align 1
bar:	
@tlscall global, manually relaxed to IE
	ldr	r0, 1f
2:	add	r0, pc, r0
	ldr	r0, [r0]
	nop
	.p2align 2
1:	.word	gd1(gottpoff) + (. - 2b - 4)
	
	.p2align 1
@tlscall global, should relax to IE
	ldr	r0, 1f
2:	blx	gd1(tlscall)
	nop
	.p2align 2
1:	.word	gd1(tlsdesc) + (. - 2b + 1)

	.p2align 1
@tlscall global, should relax to IE
	ldr	r0, 1f
2:	blx	r1(tlscall)
	nop
	.p2align 2
1:	.word	r1(tlsdesc) + (. - 2b + 1)

	.p2align 1
@tlscall local, manually relaxed to LE
	ldr	r0, 1f
	nop
	nop
	.p2align 2
1:	.word	ld1(tpoff)

	.p2align 1
@tlscall local, should relax to LE
	ldr	r0, 1f
2:	blx	ld1(tlscall)
	nop
	.p2align 2
1:	.word	ld1(tlsdesc) + (. - 2b + 1)

	.p2align 1
@tlscall local, should relax to LE
	ldr	r0, 1f
2:	blx	r0(tlscall)
	nop
	.p2align 2
1:	.word	r0(tlsdesc) + (. - 2b + 1)

	.p2align 1
@open coded global, manually relaxed to IE
	ldr	r0, 1f
2:
	add	r0, pc
	ldr	r1, [r0]
	mov	r0, r1
	nop
	.p2align 2
1:	.word	gd1(gottpoff) + (. - 2b - 4)

	.p2align 1
@open coded global, should relax to IE
	ldr	r0, 1f
2:
.tlsdescseq gd1
	add	r0, pc
.tlsdescseq gd1
	ldr	r1, [r0,#4]
.tlsdescseq gd1
	blx	r1
	nop
	.p2align 2
1:	.word	gd1(tlsdesc) + (. - 2b + 1)

	.p2align 1
@open coded local, manually relaxed to LE
	ldr	r0, 1f
2:
	nop
	nop
	nop
	nop
	.p2align 2
1:	.word	ld1(tpoff)
	
	.p2align 1
@open coded local, should relax to LE
	ldr	r0, 1f
2:
.tlsdescseq ld1
	add	r0, pc
.tlsdescseq ld1
	ldr	r1, [r0,#4]
.tlsdescseq ld1
	blx	r1
	nop
	.p2align 2
1:	.word	ld1(tlsdesc) + (. - 2b + 1)

	.section .tdata,"awT"
	.global	gd1
gd1:	.space 4
ld1:	.space 4
	.globl r1
r1:	.space 4
r0:	.space 4
