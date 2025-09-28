	.text
	.set	noreorder
	.space	0x1000

	.align	4
	.set	mips0
	.ent	foo
foo:
	nor	$0, $0
	bc	bar0
	nor	$0, $0
	beqzc	$2, bar0
	 nor	$0, $0
	beqz	$2, bar0
	 nor	$0, $0
	bc	bar1
	nor	$0, $0
	beqzc	$2, bar1
	 nor	$0, $0
	beqz	$2, bar1
	 nor	$0, $0
	bc	bar2
	nor	$0, $0
	beqzc	$2, bar2
	 nor	$0, $0
	beqz	$2, bar2
	 nor	$0, $0
	bc	bar3
	nor	$0, $0
	beqzc	$2, bar3
	 nor	$0, $0
	beqz	$2, bar3
	 nor	$0, $0
	bc	bar4
	nor	$0, $0
	beqzc	$2, bar4
	 nor	$0, $0
	beqz	$2, bar4
	 nor	$0, $0
	bc	bar4 + 1
	nor	$0, $0
	beqzc	$2, bar4 + 1
	 nor	$0, $0
	beqz	$2, bar4 + 1
	 nor	$0, $0
	bc	bar4 + 2
	nor	$0, $0
	beqzc	$2, bar4 + 2
	 nor	$0, $0
	beqz	$2, bar4 + 2
	 nor	$0, $0
	bc	bar4 + 3
	nor	$0, $0
	beqzc	$2, bar4 + 3
	 nor	$0, $0
	beqz	$2, bar4 + 3
	 nor	$0, $0
	bc	bar4 + 4
	nor	$0, $0
	beqzc	$2, bar4 + 4
	 nor	$0, $0
	beqz	$2, bar4 + 4
	 nor	$0, $0
	bc	bar16
	nor	$0, $0
	beqzc	$2, bar16
	 nor	$0, $0
	beqz	$2, bar16
	 nor	$0, $0
	bc	bar17
	nor	$0, $0
	beqzc	$2, bar17
	 nor	$0, $0
	beqz	$2, bar17
	 nor	$0, $0
	bc	bar18
	nor	$0, $0
	beqzc	$2, bar18
	 nor	$0, $0
	beqz	$2, bar18
	 nor	$0, $0
	bc	bar18 + 1
	nor	$0, $0
	beqzc	$2, bar18 + 1
	 nor	$0, $0
	beqz	$2, bar18 + 1
	 nor	$0, $0
	bc	bar18 + 2
	nor	$0, $0
	beqzc	$2, bar18 + 2
	 nor	$0, $0
	beqz	$2, bar18 + 2
	 nor	$0, $0
	bc	bar18 + 3
	nor	$0, $0
	beqzc	$2, bar18 + 3
	 nor	$0, $0
	beqz	$2, bar18 + 3
	 nor	$0, $0
	bc	bar18 + 4
	nor	$0, $0
	beqzc	$2, bar18 + 4
	 nor	$0, $0
	beqz	$2, bar18 + 4
	 nor	$0, $0
	jalr	$0, $ra
	 nor	$0, $0
	.end	foo
	.set	mips0

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
	.set	mips0
	obj	16
	fun	8
