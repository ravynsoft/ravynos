	.macro	i_nop
	.if	nop_type == 1
	nop	0
	.elseif	nop_type == 2
	l.nop
	.elseif	nop_type == 3
	nopr	1
	.elseif	nop_type == 4
	mov	g0, g0
	.elseif	nop_type == 5
	set	$0, $0
        .elseif nop_type == 6
        mov	%r1,%r1
	.else
	nop
	.endif
	.endm

	.text
	.org	0x20
	.globl	foo
foo:
	i_nop
	.org	0x10
	.globl	bar
bar:
	i_nop
