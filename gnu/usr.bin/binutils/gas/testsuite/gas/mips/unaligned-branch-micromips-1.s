	.text
	.set	noreorder
	.space	0x1000

	.align	4
	.set	micromips
	.ent	foo
foo:
	not	$2, $3
	bal	bar0
	 not	$2, $3
	bal	bar1
	 not	$2, $3
	bal	bar2
	 not	$2, $3
	bal	bar3
	 not	$2, $3
	bal	bar4
	 not	$2, $3
	bal	bar4 + 1
	 not	$2, $3
	bal	bar4 + 2
	 not	$2, $3
	bal	bar4 + 3
	 not	$2, $3
	bal	bar4 + 4
	 not	$2, $3
	bal	bar16
	 not	$2, $3
	bal	bar17
	 not	$2, $3
	bal	bar18
	 not	$2, $3
	bal	bar18 + 1
	 not	$2, $3
	bal	bar18 + 2
	 not	$2, $3
	bal	bar18 + 3
	 not	$2, $3
	bal	bar18 + 4
	 not	$2, $3
	bals	bar0
	 not	$2, $3
	bals	bar1
	 not	$2, $3
	bals	bar2
	 not	$2, $3
	bals	bar3
	 not	$2, $3
	bals	bar4
	 not	$2, $3
	bals	bar4 + 1
	 not	$2, $3
	bals	bar4 + 2
	 not	$2, $3
	bals	bar4 + 3
	 not	$2, $3
	bals	bar4 + 4
	 not	$2, $3
	bals	bar16
	 not	$2, $3
	bals	bar17
	 not	$2, $3
	bals	bar18
	 not	$2, $3
	bals	bar18 + 1
	 not	$2, $3
	bals	bar18 + 2
	 not	$2, $3
	bals	bar18 + 3
	 not	$2, $3
	bals	bar18 + 4
	 not	$2, $3
	bne	$2, $3, bar0
	 not	$2, $3
	bne	$2, $3, bar1
	 not	$2, $3
	bne	$2, $3, bar2
	 not	$2, $3
	bne	$2, $3, bar3
	 not	$2, $3
	bne	$2, $3, bar4
	 not	$2, $3
	bne	$2, $3, bar4 + 1
	 not	$2, $3
	bne	$2, $3, bar4 + 2
	 not	$2, $3
	bne	$2, $3, bar4 + 3
	 not	$2, $3
	bne	$2, $3, bar4 + 4
	 not	$2, $3
	bne	$2, $3, bar16
	 not	$2, $3
	bne	$2, $3, bar17
	 not	$2, $3
	bne	$2, $3, bar18
	 not	$2, $3
	bne	$2, $3, bar18 + 1
	 not	$2, $3
	bne	$2, $3, bar18 + 2
	 not	$2, $3
	bne	$2, $3, bar18 + 3
	 not	$2, $3
	bne	$2, $3, bar18 + 4
	 not	$2, $3
	b	bar0
	 not	$2, $3
	b	bar1
	 not	$2, $3
	b	bar2
	 not	$2, $3
	b	bar3
	 not	$2, $3
	b	bar4
	 not	$2, $3
	b	bar4 + 1
	 not	$2, $3
	b	bar4 + 2
	 not	$2, $3
	b	bar4 + 3
	 not	$2, $3
	b	bar4 + 4
	 not	$2, $3
	b	bar16
	 not	$2, $3
	b	bar17
	 not	$2, $3
	b	bar18
	 not	$2, $3
	b	bar18 + 1
	 not	$2, $3
	b	bar18 + 2
	 not	$2, $3
	b	bar18 + 3
	 not	$2, $3
	b	bar18 + 4
	 not	$2, $3
	bnez	$2, bar0
	 not	$2, $3
	bnez	$2, bar1
	 not	$2, $3
	bnez	$2, bar2
	 not	$2, $3
	bnez	$2, bar3
	 not	$2, $3
	bnez	$2, bar4
	 not	$2, $3
	bnez	$2, bar4 + 1
	 not	$2, $3
	bnez	$2, bar4 + 2
	 not	$2, $3
	bnez	$2, bar4 + 3
	 not	$2, $3
	bnez	$2, bar4 + 4
	 not	$2, $3
	bnez	$2, bar16
	 not	$2, $3
	bnez	$2, bar17
	 not	$2, $3
	bnez	$2, bar18
	 not	$2, $3
	bnez	$2, bar18 + 1
	 not	$2, $3
	bnez	$2, bar18 + 2
	 not	$2, $3
	bnez	$2, bar18 + 3
	 not	$2, $3
	bnez	$2, bar18 + 4
	 not	$2, $3
	jalr	$0, $ra
	 not	$2, $3
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16

	.macro	obj n:req
	.type	bar\@, @object
bar\@ :
	.byte	0
	.size	bar\@, . - bar\@
	.if	\n - 1
	obj	\n - 1
	.endif
	.endm

	.macro	fun n:req
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
	obj	16
	fun	8
