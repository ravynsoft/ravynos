#NO_APP
	.text
	.align	2
	.globl	main
	.type	main, @function
	.globl	_start
	.type	_start, @function
main:
_start:
	link.w %fp,#0
	move.l %a5,-(%sp)
	move.l #_GLOBAL_OFFSET_TABLE_@GOTPC, %a5
	lea (-6, %pc, %a5), %a5
	move.l x@TLSIE(%a5),%a0
	move.l -4(%fp),%a5
	unlk %fp
	rts
	.size	main, .-main
	.section	.note.GNU-stack,"",@progbits
