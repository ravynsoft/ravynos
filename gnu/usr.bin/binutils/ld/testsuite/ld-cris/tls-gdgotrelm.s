	.text

	.ifndef r
	.set r,1
	.endif

	.macro doit
	.global	tlsdsofngdgotrelm\@
	.type	tlsdsofngdgotrelm\@,@function
	.p2align 1
tlsdsofngdgotrelm\@:
	move.w x\@:GDGOTREL16,$r10
.Lfe\@:
	.size	tlsdsofngdgotrelm\@,.Lfe\@-tlsdsofngdgotrelm\@

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
