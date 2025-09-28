#NO_APP
	.file	"got-1.c"
	.text
	.align	2
	.globl	foo
	.type	foo, @function
foo:
	link.w %fp,#0
	move.l %a5,-(%sp)
	lea (%pc, _GLOBAL_OFFSET_TABLE_@GOTPC), %a5
	move.l a@GOT(%a5),%d0
	move.l %d0,%a0
	move.l (%a0),%d0
	move.l (%sp)+,%a5
	unlk %fp
	rts
	.size	foo, .-foo
	.section	.note.GNU-stack,"",@progbits
