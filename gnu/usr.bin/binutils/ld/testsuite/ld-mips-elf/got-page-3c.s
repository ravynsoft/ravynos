	.macro	makeref,sym
	lw	$5,%got(\sym\@)($gp)
	.endm

	.globl	f3
	.ent	f3
f3:
	.rept	8000
	makeref	frob
	.endr
	.end	f3
