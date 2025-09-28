	.macro	makeref,sym
	lw	$5,%got(\sym\@)($gp)
	.endm

	.globl	f2
	.ent	f2
f2:
	.rept	8000
	makeref	bar
	.endr
	.end	f2
