	.text

	.ifndef r
	.set r,1
	.endif

	.macro doit
	.global	tlsdsofngottprelm\@
	.type	tlsdsofngottprelm\@,@function
	.p2align 1
tlsdsofngottprelm\@:
	move.w x\@:TPOFFGOT16,$r10
.Lfe\@:
	.size	tlsdsofngottprelm\@,.Lfe\@-tlsdsofngottprelm\@

	.globl x\@
	.section	.tdata,"awT",@progbits
	.p2align 0
	.type	x\@, @object
	.size	x\@, 1
x\@:
	.byte	40
	.previous
	.endm

	.rept r
	doit
	.endr
