	.text
	.set	noreorder
	.space	0x1000

	.align	4
	.ent	foo
foo:
	nor	$0, $0
	bal	bar0
	 nor	$0, $0
	b	bar0
	 nor	$0, $0
	bne	$2, $3, bar0
	 nor	$0, $0
	bal	bar1
	 nor	$0, $0
	b	bar1
	 nor	$0, $0
	bne	$2, $3, bar1
	 nor	$0, $0
	bal	bar2
	 nor	$0, $0
	b	bar2
	 nor	$0, $0
	bne	$2, $3, bar2
	 nor	$0, $0
	bal	bar3
	 nor	$0, $0
	b	bar3
	 nor	$0, $0
	bne	$2, $3, bar3
	 nor	$0, $0
	bal	bar4
	 nor	$0, $0
	b	bar4
	 nor	$0, $0
	bne	$2, $3, bar4
	 nor	$0, $0
	bal	bar4 + 1
	 nor	$0, $0
	b	bar4 + 1
	 nor	$0, $0
	bne	$2, $3, bar4 + 1
	 nor	$0, $0
	bal	bar4 + 2
	 nor	$0, $0
	b	bar4 + 2
	 nor	$0, $0
	bne	$2, $3, bar4 + 2
	 nor	$0, $0
	bal	bar4 + 3
	 nor	$0, $0
	b	bar4 + 3
	 nor	$0, $0
	bne	$2, $3, bar4 + 3
	 nor	$0, $0
	bal	bar4 + 4
	 nor	$0, $0
	b	bar4 + 4
	 nor	$0, $0
	bne	$2, $3, bar4 + 4
	 nor	$0, $0
	bal	bar16
	 nor	$0, $0
	b	bar16
	 nor	$0, $0
	bne	$2, $3, bar16
	 nor	$0, $0
	bal	bar17
	 nor	$0, $0
	b	bar17
	 nor	$0, $0
	bne	$2, $3, bar17
	 nor	$0, $0
	bal	bar18
	 nor	$0, $0
	b	bar18
	 nor	$0, $0
	bne	$2, $3, bar18
	 nor	$0, $0
	bal	bar18 + 1
	 nor	$0, $0
	b	bar18 + 1
	 nor	$0, $0
	bne	$2, $3, bar18 + 1
	 nor	$0, $0
	bal	bar18 + 2
	 nor	$0, $0
	b	bar18 + 2
	 nor	$0, $0
	bne	$2, $3, bar18 + 2
	 nor	$0, $0
	bal	bar18 + 3
	 nor	$0, $0
	b	bar18 + 3
	 nor	$0, $0
	bne	$2, $3, bar18 + 3
	 nor	$0, $0
	bal	bar18 + 4
	 nor	$0, $0
	b	bar18 + 4
	 nor	$0, $0
	bne	$2, $3, bar18 + 4
	 nor	$0, $0
	jalr	$0, $ra
	 nor	$0, $0
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16

	.macro	obj n:req
	.globl	bar\@
	.type	bar\@, @object
bar\@ :
	.byte	0
	.size	bar\@, . - bar\@
	.if	\n - 1
	obj	\n - 1
	.endif
	.endm

	.macro	fun n:req
	.globl	bar\@
	.type	bar\@, @function
bar\@ :
	.insn
	.hword	0
	.size	bar\@, . - bar\@
	.if	\n - 1
	fun	\n - 1
	.endif
	.endm

	.align	4
	.set	micromips
	obj	16
	fun	8
