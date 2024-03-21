	.text
	.set	noreorder
	.space	0x1000

	.align	4
	.ent	foo
foo:
	nor	$0, $0
	jalx	bar0
	 nor	$0, $0
	jal	bar0
	 nor	$0, $0
	j	bar0
	 nor	$0, $0
	jalx	bar1
	 nor	$0, $0
	jal	bar1
	 nor	$0, $0
	j	bar1
	 nor	$0, $0
	jalx	bar2
	 nor	$0, $0
	jal	bar2
	 nor	$0, $0
	j	bar2
	 nor	$0, $0
	jalx	bar3
	 nor	$0, $0
	jal	bar3
	 nor	$0, $0
	j	bar3
	 nor	$0, $0
	jalx	bar4
	 nor	$0, $0
	jal	bar4
	 nor	$0, $0
	j	bar4
	 nor	$0, $0
	jalx	bar4 + 1
	 nor	$0, $0
	jal	bar4 + 1
	 nor	$0, $0
	j	bar4 + 1
	 nor	$0, $0
	jalx	bar4 + 2
	 nor	$0, $0
	jal	bar4 + 2
	 nor	$0, $0
	j	bar4 + 2
	 nor	$0, $0
	jalx	bar4 + 3
	 nor	$0, $0
	jal	bar4 + 3
	 nor	$0, $0
	j	bar4 + 3
	 nor	$0, $0
	jalx	bar4 + 4
	 nor	$0, $0
	jal	bar4 + 4
	 nor	$0, $0
	j	bar4 + 4
	 nor	$0, $0
	jalx	bar16
	 nor	$0, $0
	jal	bar16
	 nor	$0, $0
	j	bar16
	 nor	$0, $0
	jalx	bar17
	 nor	$0, $0
	jal	bar17
	 nor	$0, $0
	j	bar17
	 nor	$0, $0
	jalx	bar18
	 nor	$0, $0
	jal	bar18
	 nor	$0, $0
	j	bar18
	 nor	$0, $0
	jalx	bar18 + 1
	 nor	$0, $0
	jal	bar18 + 1
	 nor	$0, $0
	j	bar18 + 1
	 nor	$0, $0
	jalx	bar18 + 2
	 nor	$0, $0
	jal	bar18 + 2
	 nor	$0, $0
	j	bar18 + 2
	 nor	$0, $0
	jalx	bar18 + 3
	 nor	$0, $0
	jal	bar18 + 3
	 nor	$0, $0
	j	bar18 + 3
	 nor	$0, $0
	jalx	bar18 + 4
	 nor	$0, $0
	jal	bar18 + 4
	 nor	$0, $0
	j	bar18 + 4
	 nor	$0, $0
	jalr	$0, $ra
	 nor	$0, $0
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
	.set	micromips
	obj	16
	fun	8
